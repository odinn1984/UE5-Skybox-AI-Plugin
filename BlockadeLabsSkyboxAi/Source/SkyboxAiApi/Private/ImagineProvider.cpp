#include "ImagineProvider.h"
#include "SKyboxAiHttpClient.h"

UImagineProvider::UImagineProvider()
{
  SetClient(CreateDefaultSubobject<USKyboxAiHttpClient>("APIClient"));
}

void UImagineProvider::SetClient(USKyboxAiHttpClient *InAPIClient)
{
  ApiClient = InAPIClient;

  // ReSharper disable once CppExpressionWithoutSideEffects
  IsClientValid();
}

void UImagineProvider::GetRequestsObfuscatedId(const FString Id, FGetRequestsObfuscatedIdCallback Callback) const
{
  if (!IsClientValid()) return;

  ApiClient->MakeAPIRequest(
    TEXT("/imagine/requests/obfuscated-id/" + Id),
    FSkyboxAiHttpHeaders(),
    TEXT(""),
    [this, Callback](const FString &Body, int StatusCode, bool bConnectedSuccessfully)
    {
      FImagineGetExportsResponse Response;
      ApiClient->DeserializeJsonToUStruct<FImagineGetExportsResponse>(Body, &Response);
      Callback(&Response, StatusCode, bConnectedSuccessfully);
    }
    );
}

bool UImagineProvider::IsClientValid() const
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
