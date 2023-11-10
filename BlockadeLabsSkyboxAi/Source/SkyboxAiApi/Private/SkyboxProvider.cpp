#include "SkyboxProvider.h"

#include "BlockadeLabsSkyboxAiSettings.h"
#include "SKyboxAiHttpClient.h"

USkyboxProvider::USkyboxProvider()
{
  SetClient(CreateDefaultSubobject<USKyboxAiHttpClient>("APIClient"));
}

void USkyboxProvider::SetClient(USKyboxAiHttpClient *InAPIClient)
{
  ApiClient = InAPIClient;

  // ReSharper disable once CppExpressionWithoutSideEffects
  IsClientValid();
}

void USkyboxProvider::Post(const FSkyboxGenerateRequest &Data, FPostCallback Callback) const
{
  if (!IsClientValid()) return;

  FSkyboxAiHttpHeaders Headers = FSkyboxAiHttpHeaders();
  Headers.Method = SkyboxAiHttpClientDefinitions::HTTPVerbs::GPost;

  const FString Endpoint = TEXT("/skybox/");

  if (const FString *BodyPtr = USKyboxAiHttpClient::SerializeJson<FSkyboxGenerateRequest>(Data); BodyPtr == nullptr)
  {
    const FString ErrorMessage = TEXT("Invalid request body");
    constexpr int StatusCode = 422;

    FMessageLog(SkyboxAiHttpClientDefinitions::GMessageLogName)
      .Message(EMessageSeverity::Error, FText::FromString(TEXT("Post Request Failed")))
      ->AddToken(FTextToken::Create(FText::FromString(Endpoint)))
      ->AddToken(FTextToken::Create(FText::FromString(FString::SanitizeFloat(StatusCode))))
      ->AddToken(FTextToken::Create(FText::FromString(ErrorMessage)));

    Callback(nullptr, StatusCode, true);
    return;
  }

  ApiClient->MakeAPIRequest(
    Endpoint,
    Headers,
    TEXT(""),
    [this, Callback](const FString &Body, int StatusCode, bool bConnectedSuccessfully)
    {
      FSkyboxGenerateResponse *Response = USKyboxAiHttpClient::DeserializeJsonToUStruct<FSkyboxGenerateResponse>(Body);
      Callback(Response, StatusCode, bConnectedSuccessfully);
    }
    );
}

void USkyboxProvider::GetStyles(FGetStylesCallback Callback) const
{
  if (!IsClientValid()) return;

  ApiClient->MakeAPIRequest(
    TEXT("/skybox/styles"),
    FSkyboxAiHttpHeaders(),
    TEXT(""),
    [this, Callback](const FString &Body, int StatusCode, bool bConnectedSuccessfully)
    {
      FSkyboxAiStyles Styles;
      const FString SanitizedBody = Body.Replace(TEXT("\"max-char\""), TEXT("\"max_char\""))
                                        .Replace(
                                          TEXT("\"negative-text-max-char\""),
                                          TEXT("\"negative_text_max_char\"")
                                          );

      for (TArray<FSkyboxStyle> Response = USKyboxAiHttpClient::DeserializeJsonToUStructArray<FSkyboxStyle>(
             SanitizedBody
             ); FSkyboxStyle &Item : Response)
      {
        if (!ShouldShowPremiumContent() && Item.premium == 1) continue;
        Styles.Add(Item.id, FSkyboxListEntry(Item.name, Item.max_char, Item.negative_text_max_char));
      }

      Callback(Styles, StatusCode, bConnectedSuccessfully);
    }
    );
}

void USkyboxProvider::GetExports(FGetExportsCallback Callback) const
{
  if (!IsClientValid()) return;

  ApiClient->MakeAPIRequest(
    TEXT("/skybox/export"),
    FSkyboxAiHttpHeaders(),
    TEXT(""),
    [this, Callback](const FString &Body, int StatusCode, bool bConnectedSuccessfully)
    {
      FSkyboxAiExportTypes Types;

      for (TArray<FSkyboxExportType> Response = USKyboxAiHttpClient::DeserializeJsonToUStructArray<FSkyboxExportType>(
             Body
             ); FSkyboxExportType &Type : Response)
      {
        if (!ShouldShowPremiumContent() && Type.premium_feature) continue;
        Types.Add(Type.id, Type.name);
      }

      Callback(Types, StatusCode, bConnectedSuccessfully);
    }
    );
}

void USkyboxProvider::GetExport(const FString Id, FGetExportCallback Callback) const
{
  if (!IsClientValid()) return;

  ApiClient->MakeAPIRequest(
    TEXT("/skybox/export/" + Id),
    FSkyboxAiHttpHeaders(),
    TEXT(""),
    [this, Callback](const FString &Body, int StatusCode, bool bConnectedSuccessfully)
    {
      FSkyboxExportResponse *Response = USKyboxAiHttpClient::DeserializeJsonToUStruct<FSkyboxExportResponse>(Body);
      Callback(Response, StatusCode, bConnectedSuccessfully);
    }
    );
}

void USkyboxProvider::PostExport(const FSkyboxExportRequest &Data, FPostExportCallback Callback) const
{
  if (!IsClientValid()) return;

  FSkyboxAiHttpHeaders Headers = FSkyboxAiHttpHeaders();
  Headers.Method = SkyboxAiHttpClientDefinitions::HTTPVerbs::GPost;

  const FString Endpoint = TEXT("/skybox/export/");

  if (const FString *BodyPtr = USKyboxAiHttpClient::SerializeJson<FSkyboxExportRequest>(Data); BodyPtr == nullptr)
  {
    const FString ErrorMessage = TEXT("Invalid request body");
    constexpr int StatusCode = 422;

    FMessageLog(SkyboxAiHttpClientDefinitions::GMessageLogName)
      .Message(EMessageSeverity::Error, FText::FromString(TEXT("PostExport Request Failed")))
      ->AddToken(FTextToken::Create(FText::FromString(Endpoint)))
      ->AddToken(FTextToken::Create(FText::FromString(FString::SanitizeFloat(StatusCode))))
      ->AddToken(FTextToken::Create(FText::FromString(ErrorMessage)));

    Callback(nullptr, StatusCode, true);
    return;
  }

  ApiClient->MakeAPIRequest(
    Endpoint,
    Headers,
    TEXT(""),
    [this, Callback](const FString &Body, int StatusCode, bool bConnectedSuccessfully)
    {
      FSkyboxExportResponse *Response = USKyboxAiHttpClient::DeserializeJsonToUStruct<FSkyboxExportResponse>(Body);
      Callback(Response, StatusCode, bConnectedSuccessfully);
    }
    );
}

bool USkyboxProvider::IsClientValid() const
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

bool USkyboxProvider::ShouldShowPremiumContent() const
{
  const UBlockadeLabsSkyboxAiSettings *EditorSettings = GetMutableDefault<UBlockadeLabsSkyboxAiSettings>();
  return EditorSettings->bEnablePremiumContent;
}
