#include "SkyboxAiApi.h"
#include "SkyboxProvider.h"
#include "ImagineProvider.h"
#include "BlockadeLabsSkyboxAiSettings.h"
#include "HttpModule.h"
#include "SKyboxAiHttpClient.h"
#include "Interfaces/IHttpResponse.h"

DEFINE_LOG_CATEGORY(SkyboxAiApi);

USkyboxApi::USkyboxApi()
{
  ApiClient = CreateDefaultSubobject<USKyboxAiHttpClient>("APIClient");

  SkyboxProvider = CreateDefaultSubobject<USkyboxProvider>(TEXT("SkyboxProvider"));
  SkyboxProvider->SetClient(ApiClient);

  ImagineProvider = CreateDefaultSubobject<UImagineProvider>(TEXT("ImagineProvider"));
  ImagineProvider->SetClient(ApiClient);
}

void USkyboxApi::SaveExportedImage(const FString &ImageUrl, TFunction<void(bool bSuccess)> Callback) const
{
  if (!IsClientValid()) return;

  const UBlockadeLabsSkyboxAiSettings *EditorSettings = GetMutableDefault<UBlockadeLabsSkyboxAiSettings>();
  const FString SaveDirectory = FPaths::ProjectContentDir() + EditorSettings->SaveDirectory;

  if (!FPaths::DirectoryExists(SaveDirectory))
  {
    FPlatformFileManager::Get().GetPlatformFile().CreateDirectoryTree(*SaveDirectory);
  }

  FString FileName = FPaths::GetCleanFilename(ImageUrl);

  const int32 QueryParamIndex = FileName.Find(TEXT("?ver="), ESearchCase::IgnoreCase, ESearchDir::FromEnd);
  if (QueryParamIndex != INDEX_NONE) FileName = FileName.Left(QueryParamIndex);

  const FString SavePath = FPaths::Combine(SaveDirectory, FileName);

  DownloadImage(
    ImageUrl,
    [this, SavePath, Callback](TArray<uint8> ExportedImage, bool bSuccess)
    {
      if (bSuccess) FFileHelper::SaveArrayToFile(ExportedImage, *SavePath);
      Callback(bSuccess);
    }
    );
}

bool USkyboxApi::IsClientValid() const
{
  if (ApiClient == nullptr)
  {
    FMessageLog(SkyboxAiHttpClientDefinitions::GMessageLogName).Error(
      FText::FromString(TEXT("Client wasn't initialized"))
      );
    return false;
  }

  return true;
}

void USkyboxApi::DownloadImage(
  const FString &ImageUrl,
  TFunction<void(TArray<uint8> ExportedImage, bool bSuccess)> Callback) const
{
  TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();
  HttpRequest->OnProcessRequestComplete().BindLambda(
    [Callback](FHttpRequestPtr Req, FHttpResponsePtr Res, bool bConnectedSuccessfully)
    {
      if (bConnectedSuccessfully && Res.IsValid() && Res->GetContentLength() > 0)
      {
        Callback(Res->GetContent(), true);
      }
      else
      {
        Callback(TArray<uint8>(), false);
      }
    }
    );

  HttpRequest->SetURL(ImageUrl);
  HttpRequest->SetVerb(SkyboxAiHttpClientDefinitions::HTTPVerbs::GGet);
  HttpRequest->ProcessRequest();
}
