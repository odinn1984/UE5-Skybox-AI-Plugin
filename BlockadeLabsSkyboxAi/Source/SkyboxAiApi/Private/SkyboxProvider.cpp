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

  const FString Endpoint = TEXT("/skybox");
  FString SanitizedBody;

  if (!RemovePostUnsetFields(Data, &SanitizedBody))
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

  FSkyboxAiHttpHeaders Headers = FSkyboxAiHttpHeaders();
  Headers.Method = SkyboxAiHttpClientDefinitions::HTTPVerbs::GPost;

  ApiClient->MakeAPIRequest(
    Endpoint,
    Headers,
    SanitizedBody,
    [this, Callback](const FString &Body, int StatusCode, bool bConnectedSuccessfully)
    {
      FSkyboxGenerateResponse Response;

      if (!ApiClient->DeserializeJsonToUStruct<FSkyboxGenerateResponse>(Body, &Response))
      {
        Callback(nullptr, StatusCode, bConnectedSuccessfully);
      }
      else
      {
        Callback(&Response, StatusCode, bConnectedSuccessfully);
      }
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
      const FString SanitizedBody = GetStylesResponseKeysHyphensToUnderscore(Body);

      TArray<FSkyboxStyle> Response;

      if (ApiClient->DeserializeJsonToUStructArray<FSkyboxStyle>(SanitizedBody, &Response))
      {
        for (FSkyboxStyle &Item : Response)
        {
          if (!ShouldShowPremiumContent() && Item.premium == 1) continue;
          Styles.Add(Item.id, FSkyboxListEntry(Item.name, Item.max_char, Item.negative_text_max_char));
        }
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
      TArray<FSkyboxExportType> Response;

      if (ApiClient->DeserializeJsonToUStructArray<FSkyboxExportType>(Body, &Response))
      {
        for (FSkyboxExportType &Type : Response)
        {
          if (!ShouldShowPremiumContent() && Type.premium_feature) continue;
          Types.Add(Type.id, Type.name);
        }
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
      FSkyboxExportResponse Response;

      if (!ApiClient->DeserializeJsonToUStruct<FSkyboxExportResponse>(Body, &Response))
      {
        Callback(nullptr, StatusCode, bConnectedSuccessfully);
      }
      else
      {
        Callback(&Response, StatusCode, bConnectedSuccessfully);
      }
    }
    );
}

void USkyboxProvider::PostExport(const FSkyboxExportRequest &Data, FPostExportCallback Callback) const
{
  if (!IsClientValid()) return;

  const FString Endpoint = TEXT("/skybox/export/");
  FString BodyPtr;

  if (!ApiClient->SerializeJson<FSkyboxExportRequest>(Data, BodyPtr))
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

  FSkyboxAiHttpHeaders Headers = FSkyboxAiHttpHeaders();
  Headers.Method = SkyboxAiHttpClientDefinitions::HTTPVerbs::GPost;

  ApiClient->MakeAPIRequest(
    Endpoint,
    Headers,
    BodyPtr,
    [this, Callback](const FString &Body, int StatusCode, bool bConnectedSuccessfully)
    {
      FSkyboxExportResponse Response;

      if (!ApiClient->DeserializeJsonToUStruct<FSkyboxExportResponse>(Body, &Response))
      {
        Callback(nullptr, StatusCode, bConnectedSuccessfully);
      }
      else
      {
        Callback(&Response, StatusCode, bConnectedSuccessfully);
      }
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

bool USkyboxProvider::RemovePostUnsetFields(const FSkyboxGenerateRequest &Data, FString *OutBody) const
{
  TSharedPtr<FJsonObject> FilteredJsonObj = MakeShareable(new FJsonObject);

  if (!FJsonObjectConverter::UStructToJsonObject(
    FSkyboxGenerateRequest::StaticStruct(),
    &Data,
    FilteredJsonObj.ToSharedRef(),
    0,
    0
    ))
  {
    FMessageLog(SkyboxAiHttpClientDefinitions::GMessageLogName).Error(
      FText::FromString(TEXT("[USkyboxProvider::Post] Serialization from JSON failed"))
      );

    return false;
  }

  if (
    FilteredJsonObj->HasField(TEXT("negative_text")) &&
    FilteredJsonObj->GetStringField(TEXT("negative_text")).Equals(
      SkyboxAiHttpClientDefinitions::GIgnoreStringJsonFieldValue
      )
  )
  {
    FilteredJsonObj->RemoveField(TEXT("negative_text"));
  }

  if (
    FilteredJsonObj->HasField(TEXT("skybox_style_id")) &&
    FilteredJsonObj->GetIntegerField(TEXT("skybox_style_id")) == INDEX_NONE
  )
  {
    FilteredJsonObj->RemoveField(TEXT("skybox_style_id"));
  }

  TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(OutBody);
  return FJsonSerializer::Serialize(FilteredJsonObj.ToSharedRef(), Writer);
}

FString USkyboxProvider::GetStylesResponseKeysHyphensToUnderscore(const FString &Body) const
{
  return Body.Replace(TEXT("\"max-char\""), TEXT("\"max_char\""))
             .Replace(
               TEXT("\"negative-text-max-char\""),
               TEXT("\"negative_text_max_char\"")
               );
}
