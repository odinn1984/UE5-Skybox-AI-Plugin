#include "SKyboxAiHttpClient.h"
#include "BlockadeLabs_SkyboxAISettings.h"
#include "SkyboxProvider.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"

DEFINE_LOG_CATEGORY(SkyboxAIAPI);

USkyboxApi::USkyboxApi()
{
  UBlockadeLabs_SkyboxAISettings *EditorSettings = GetMutableDefault<UBlockadeLabs_SkyboxAISettings>();
  const FString APIKey = EditorSettings->APIKey;
  const FString APIEndpoint = EditorSettings->APIEndpoint;

  EditorSettings->OnSettingChanged().AddUObject(this, &USkyboxApi::OnSettingsChanged);

  APIClient = CreateDefaultSubobject<USKyboxAiHttpClient>(TEXT("SkyboxAiHttpClient"));
  APIClient->SetApiKey(APIKey);
  if (!APIEndpoint.Equals(TEXT(""))) APIClient->SetApiEndpoint(APIEndpoint);

  SkyboxProvider = CreateDefaultSubobject<USkyboxProvider>(TEXT("SkyboxProvider"));
  SkyboxProvider->SetClient(APIClient);
}

void USkyboxApi::OnSettingsChanged(UObject *Object, FPropertyChangedEvent &Event)
{
  APIClient->SetApiKey(CastChecked<UBlockadeLabs_SkyboxAISettings>(Object)->APIKey);
  APIClient->SetApiEndpoint(CastChecked<UBlockadeLabs_SkyboxAISettings>(Object)->APIEndpoint);
}

USKyboxAiHttpClient::USKyboxAiHttpClient()
{
  Http = &FHttpModule::Get();
}

void USKyboxAiHttpClient::SetHttpModule(FHttpModule *InHttp)
{
  Http = InHttp;
}

void USKyboxAiHttpClient::SetApiKey(FString InAPIKey)
{
  APIKey = InAPIKey;
}

void USKyboxAiHttpClient::SetApiEndpoint(FString InEndpointOverride)
{
  APIEndpoint = InEndpointOverride;
}

void USKyboxAiHttpClient::MakeAPIRequest(
  const FString &Endpoint,
  const FSkyboxAiHttpHeaders &Headers,
  const FString &Body,
  FSkyboxAiHttpCallback Callback) const
{
  const TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = Http->CreateRequest();
  const FString URL = APIEndpoint + Endpoint;

  Request->SetURL(URL);
  Request->SetVerb(Headers.Method);
  Request->SetHeader("x-api-key", APIKey);
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
  FSkyboxApiError *ParsedResponse = DeserializeJSONToUStruct<FSkyboxApiError>(Response->GetContentAsString());

  if (ParsedResponse != nullptr) ErrorMessage = ParsedResponse->error;
  if (StatusCode >= 500) Severity = EMessageSeverity::Error;

  FMessageLog(SkyboxAiHttpClient::GMessageLogName)
    .Message(Severity, FText::FromString(TEXT("HTTP Request Done")))
    ->AddToken(FTextToken::Create(FText::FromString(Response->GetURL())))
    ->AddToken(FTextToken::Create(FText::FromString(FString::SanitizeFloat(StatusCode))))
    ->AddToken(FTextToken::Create(FText::FromString(ErrorMessage)));
}
