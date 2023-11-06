#include "SkyboxProvider.h"
#include "SKyboxAiHttpClient.h"

USkyboxProvider::USkyboxProvider()
{
  SetClient(CreateDefaultSubobject<USKyboxAiHttpClient>("APIClient"));
}

void USkyboxProvider::SetClient(USKyboxAiHttpClient *InAPIClient)
{
  if (APIClient = InAPIClient; APIClient == nullptr)
  {
    FMessageLog(SkyboxAiHttpClient::GMessageLogName).Error(FText::FromString(TEXT("Client wasn't initialized")));
  }
}

void USkyboxProvider::Post(const FSkyboxGenerateRequest &Data, FPostCallback Callback) const
{
}

void USkyboxProvider::GetStyles(FGetStylesCallback Callback) const
{
  APIClient->MakeAPIRequest(
    TEXT("/skybox/styles"),
    FSkyboxAiHttpHeaders(),
    TEXT(""),
    [this, Callback](const FString &Body, int StatusCode, bool bConnectedSuccessfully)
    {
      TMap<int, FString> Styles = TMap<int, FString>();

      for (TArray<FSkyboxStyle> Response = USKyboxAiHttpClient::DeserializeJSONToUStructArray<FSkyboxStyle>(Body);
           FSkyboxStyle &Item : Response)
      {
        Styles.Add(Item.id, Item.name);
      }

      Callback(Styles, StatusCode, bConnectedSuccessfully);
    }
    );
}

void USkyboxProvider::GetExports(FGetExportsCallback Callback) const
{
  if (APIClient == nullptr)
  {
    FMessageLog(SkyboxAiHttpClient::GMessageLogName).Error(FText::FromString(TEXT("Client wasn't initialized")));
    return;
  }

  APIClient->MakeAPIRequest(
    TEXT("/skybox/export"),
    FSkyboxAiHttpHeaders(),
    TEXT(""),
    [this, Callback](const FString &Body, int StatusCode, bool bConnectedSuccessfully)
    {
      TMap<int, FString> Types = TMap<int, FString>();

      for (TArray<FSkyboxExportType> Response = USKyboxAiHttpClient::DeserializeJSONToUStructArray<FSkyboxExportType>(
             Body
             ); FSkyboxExportType &Type : Response)
      {
        Types.Add(Type.id, Type.name);
      }

      Callback(Types, StatusCode, bConnectedSuccessfully);
    }
    );
}

void USkyboxProvider::GetExport(const FString Id, FGetExportCallback Callback) const
{
  APIClient->MakeAPIRequest(
    TEXT("/skybox/export/" + Id),
    FSkyboxAiHttpHeaders(),
    TEXT(""),
    [this, Callback](const FString &Body, int StatusCode, bool bConnectedSuccessfully)
    {
      FSkyboxExportResponse *Response = USKyboxAiHttpClient::DeserializeJSONToUStruct<FSkyboxExportResponse>(Body);
      Callback(Response, StatusCode, bConnectedSuccessfully);
    }
    );
}

void USkyboxProvider::PostExport(const FSkyboxExportRequest &Data, FPostExportCallback Callback) const
{
}
