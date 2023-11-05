#include "SkyboxProvider.h"
#include "SKyboxAiHttpClient.h"

USkyboxProvider::USkyboxProvider()
{
  APIClient = CreateDefaultSubobject<USKyboxAiHttpClient>("APIClient");
}

void USkyboxProvider::SetClient(USKyboxAiHttpClient *InAPIClient)
{
  APIClient = InAPIClient;
}

void USkyboxProvider::Categories() const
{
  if (APIClient == nullptr)
  {
    FMessageLog(SkyboxAiHttpClient::GMessageLogName).Error(FText::FromString(TEXT("Client wasn't initialized")));
    return;
  }

  APIClient->MakeAPIRequest(
    TEXT("/skybox/styles"),
    FSkyboxAiHttpHeaders(),
    TEXT(""),
    [this](const FString &Body)
    {
      TMap<int, FString> Styles = TMap<int, FString>();

      for (TArray<FSkyboxStyle> Response = USKyboxAiHttpClient::DeserializeJSONToUStructArray<FSkyboxStyle>(Body);
           FSkyboxStyle &Item : Response)
      {
        Styles.Add(Item.id, Item.name);
      }

      OnCategoriesRetrieved.Broadcast(Styles);
    }
    );
}

void USkyboxProvider::ExportTypes() const
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
    [this](const FString &Body)
    {
      TMap<int, FString> Types = TMap<int, FString>();

      for (TArray<FSkyboxExportType> Response = USKyboxAiHttpClient::DeserializeJSONToUStructArray<FSkyboxExportType>(
             Body
             ); FSkyboxExportType &Type : Response)
      {
        Types.Add(Type.id, Type.name);
      }

      OnExportTypesRetrieved.Broadcast(Types);
    }
    );
}
