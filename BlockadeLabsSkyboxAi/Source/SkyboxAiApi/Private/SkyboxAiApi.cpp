#include "SkyboxAiApi.h"
#include "SkyboxProvider.h"
#include "ImagineProvider.h"
#include "BlockadeLabsSkyboxAiSettings.h"
#include "SKyboxAiHttpClient.h"

DEFINE_LOG_CATEGORY(SkyboxAiApi);

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
    FMessageLog(SkyboxAiHttpClientDefinitions::GMessageLogName).Error(FText::FromString(TEXT("Client wasn't initialized")));
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
