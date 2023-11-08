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

void UImagineProvider::GetRequests(const FString Id, FGetRequestsCallback Callback) const
{
  if (!IsClientValid()) return;
}

bool UImagineProvider::IsClientValid() const
{
  if (ApiClient == nullptr)
  {
    FMessageLog(SkyboxAiHttpClient::GMessageLogName).Error(FText::FromString(TEXT("Client wasn't initialized")));
    return false;
  }

  return true;
}
