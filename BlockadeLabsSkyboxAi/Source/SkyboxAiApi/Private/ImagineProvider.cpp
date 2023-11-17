#include "ImagineProvider.h"
#include "SKyboxAiHttpClient.h"

UImagineProvider::UImagineProvider()
{
  SetClient(CreateDefaultSubobject<USKyboxAiHttpClient>("APIClient"));
}

void UImagineProvider::SetClient(USKyboxAiHttpClient* InAPIClient)
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
    [this, Callback](const FString& Body, int StatusCode, bool bConnectedSuccessfully)
    {
      FImagineGetExportsResponse Response;
      ApiClient->DeserializeJsonToUStruct<FImagineGetExportsResponse>(Body, &Response);
      Callback(&Response.request, StatusCode, bConnectedSuccessfully);
    }
  );
}

void UImagineProvider::GetRequestsObfuscatedId(const FString Id, FGetRequestsObfuscatedIdCallback Callback) const
{
  if (!IsClientValid()) return;

  if (Id.IsEmpty())
  {
    FImagineGetExportsResponse Response;
    Response.request.status = TEXT("failed");
    Response.request.error_message = TEXT("Invalid ID provided");

    return Callback(&Response.request, 404, true);
  }

  ApiClient->MakeAPIRequest(
    TEXT("/imagine/requests/obfuscated-id/" + Id),
    FSkyboxAiHttpHeaders(),
    TEXT(""),
    [this, Callback](const FString& Body, int StatusCode, bool bConnectedSuccessfully)
    {
      FImagineGetExportsResponse Response;
      ApiClient->DeserializeJsonToUStruct<FImagineGetExportsResponse>(Body, &Response);
      Callback(&Response.request, StatusCode, bConnectedSuccessfully);
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
