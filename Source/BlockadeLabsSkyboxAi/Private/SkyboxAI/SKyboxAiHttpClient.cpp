#include "SKyboxAiHttpClient.h"
#include "BlockadeLabsSkyboxAiSettings.h"
#include "SkyboxProvider.h"
#include "HttpModule.h"
#include "ImagineProvider.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"

DEFINE_LOG_CATEGORY(SkyboxAiAPI);

USkyboxApi::USkyboxApi()
{
  ApiClient = CreateDefaultSubobject<USKyboxAiHttpClient>("APIClient");

  SkyboxProvider = CreateDefaultSubobject<USkyboxProvider>(TEXT("SkyboxProvider"));
  SkyboxProvider->SetClient(ApiClient);

  ImagineProvider = CreateDefaultSubobject<UImagineProvider>(TEXT("ImagineProvider"));
  ImagineProvider->SetClient(ApiClient);
}

void USkyboxApi::SaveExportedImage(const FString &Id) const
{
  if (!IsClientValid()) return;

  Imagine()->GetRequests(
    Id,
    [this](FImagineGetExportsResponse *Response, int StatusCode, bool bConnectedSuccessfully)
    {
      if (!ValidateExportedImageCall(Response, StatusCode, bConnectedSuccessfully)) return;

      const UBlockadeLabsSkyboxAiSettings *EditorSettings = GetMutableDefault<UBlockadeLabsSkyboxAiSettings>();
      const FString SaveDirectory = EditorSettings->SaveDirectory;
      const FString ImageUrl = Response->request.file_url;
      const FString Title = Response->request.title;
      const FString SavePath = FPaths::Combine(SaveDirectory, Title);
      const FString SaveDir = FPaths::GetPath(SavePath);

      if (!FPaths::DirectoryExists(SaveDir))
      {
        FPlatformFileManager::Get().GetPlatformFile().CreateDirectoryTree(*SaveDir);
      }

      // DownloadImage(ImageUrl, [this, SavePath](TArray<uint8> ExportedImage)
      // {
      //   FFileHelper::SaveArrayToFile(ExportedImage, *SavePath);
      // });
    }
    );
}

bool USkyboxApi::IsClientValid() const
{
  if (ApiClient == nullptr)
  {
    FMessageLog(SkyboxAiHttpClient::GMessageLogName).Error(FText::FromString(TEXT("Client wasn't initialized")));
    return false;
  }

  return true;
}

bool USkyboxApi::ValidateExportedImageCall(
  FImagineGetExportsResponse *Response,
  int StatusCode,
  bool bConnectedSuccessfully) const
{
  if (!bConnectedSuccessfully || Response == nullptr || StatusCode >= 300)
  {
    return false;
  }

  if (Response->request.status != TEXT("complete"))
  {
    return false;
  }

  return true;
}

USKyboxAiHttpClient::USKyboxAiHttpClient()
{
  Http = &FHttpModule::Get();
}

void USKyboxAiHttpClient::SetHttpModule(FHttpModule *InHttp)
{
  Http = InHttp;
}

void USKyboxAiHttpClient::MakeAPIRequest(
  const FString &Endpoint,
  const FSkyboxAiHttpHeaders &Headers,
  const FString &Body,
  FSkyboxAiHttpCallback Callback) const
{
  const UBlockadeLabsSkyboxAiSettings *EditorSettings = GetMutableDefault<UBlockadeLabsSkyboxAiSettings>();
  const FString ApiKey = EditorSettings->ApiKey;
  const FString ApiEndpoint = EditorSettings->ApiEndpoint;

  const TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = Http->CreateRequest();
  const FString URL = ApiEndpoint + Endpoint;

  Request->SetURL(URL);
  Request->SetVerb(Headers.Method);
  Request->SetHeader("x-api-key", ApiKey);
  Request->SetHeader("accept", Headers.Accept);
  Request->SetHeader("Content-Type", Headers.ContentType);

  if (!Body.Equals(TEXT(""))) Request->SetContentAsString(Body);

  Request->ProcessRequest();
  Request->OnProcessRequestComplete().BindLambda(
    [this, Callback](FHttpRequestPtr Req, FHttpResponsePtr Res, bool bConnectedSuccessfully)
    {
      FString ResBody;
      int StatusCode = 0;

      if (!bConnectedSuccessfully || !Res.IsValid())
      {
        ResBody = TEXT("Failed to connect to the server");
        FMessageLog(SkyboxAiHttpClient::GMessageLogName).
          Error(FText::FromString(ResBody))->
          AddToken(FTextToken::Create(FText::FromString(Req->GetURL())));
      }
      else
      {
        ResBody = Res.Get()->GetContentAsString();
        StatusCode = Res.Get()->GetResponseCode();

        if (StatusCode >= 300) HandleHttpResponse(Res.ToSharedRef());
      }

      Callback(ResBody, StatusCode, bConnectedSuccessfully);
    }
    );
}

void USKyboxAiHttpClient::HandleHttpResponse(FHttpResponseRef Response) const
{
  const int StatusCode = Response->GetResponseCode();

  if (StatusCode < 300) return;

  EMessageSeverity::Type Severity = EMessageSeverity::Warning;
  FString ErrorMessage = TEXT("Unexpected error occurred");
  FSkyboxApiError *ParsedResponse = DeserializeJsonToUStruct<FSkyboxApiError>(Response->GetContentAsString());

  if (ParsedResponse != nullptr) ErrorMessage = ParsedResponse->error;
  if (StatusCode >= 500) Severity = EMessageSeverity::Error;

  FMessageLog(SkyboxAiHttpClient::GMessageLogName)
    .Message(Severity, FText::FromString(TEXT("HTTP Request Done")))
    ->AddToken(FTextToken::Create(FText::FromString(Response->GetURL())))
    ->AddToken(FTextToken::Create(FText::FromString(FString::SanitizeFloat(StatusCode))))
    ->AddToken(FTextToken::Create(FText::FromString(ErrorMessage)));
}
