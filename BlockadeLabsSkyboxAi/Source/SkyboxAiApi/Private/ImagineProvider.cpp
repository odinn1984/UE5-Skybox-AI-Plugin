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

void UImagineProvider::GetRequests(const uint32 Id, FGetRequestsObfuscatedIdCallback Callback) const
{
  if (!IsClientValid()) return;

  ApiClient->MakeAPIRequest(
    TEXT("/imagine/requests/" + FString::FromInt(Id)),
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

void UImagineProvider::GetRequestsObfuscatedId(const FString Id, FGetRequestsObfuscatedIdCallback Callback) const
{
  if (!IsClientValid()) return;

  FImagineGetExportsResponse Response;

  if (Id.IsEmpty())
  {
    Response.request.status = TEXT("failed");
    Response.request.error_message = TEXT("Invalid ID provided");

    return Callback(&Response, 422, true);
  }

  ApiClient->MakeAPIRequest(
    TEXT("/imagine/requests/obfuscated-id/" + Id),
    FSkyboxAiHttpHeaders(),
    TEXT(""),
    [this, Callback, &Response](const FString &Body, int StatusCode, bool bConnectedSuccessfully)
    {
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
