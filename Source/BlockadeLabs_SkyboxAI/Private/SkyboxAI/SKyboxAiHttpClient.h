#pragma once

#include "CoreMinimal.h"
#include "JsonObjectConverter.h"
#include "SkyboxAiHttpClient.generated.h"

class USkyboxProvider;
class USKyboxAiHttpClient;

DECLARE_LOG_CATEGORY_EXTERN(SkyboxAIAPI, Log, All);

namespace SkyboxAiHttpClient
{
  const static FName GMessageLogName = TEXT("SkyboxAI");

  namespace HTTPVerbs
  {
    const static FString GGet = "GET";
    const static FString GPost = "POST";
    const static FString GPut = "PUT";
    const static FString GDelete = "DELETE";
    const static FString GHead = "HEAD";
  }

  namespace ContentTypes
  {
    const static FString GJson = TEXT("application/json");
    const static FString GFormData = TEXT("multipart/form-data");
  }
}

USTRUCT(BlueprintType)
struct FSkyboxAiHttpHeaders
{
  GENERATED_BODY()

  UPROPERTY()
  FString Method = SkyboxAiHttpClient::HTTPVerbs::GGet;

  UPROPERTY()
  FString Accept = SkyboxAiHttpClient::ContentTypes::GJson;

  UPROPERTY()
  FString ContentType = SkyboxAiHttpClient::ContentTypes::GJson;
};

UCLASS()
class USkyboxApi : public UObject
{
  GENERATED_BODY()

public:
  explicit USkyboxApi();

  FORCEINLINE USkyboxProvider *Skybox() const { return SkyboxProvider; }

protected:
  virtual void OnSettingsChanged(UObject *Object, FPropertyChangedEvent &Event);

private:
  UPROPERTY()
  TObjectPtr<USKyboxAiHttpClient> APIClient;

  UPROPERTY()
  TObjectPtr<USkyboxProvider> SkyboxProvider;
};

UCLASS()
class USKyboxAiHttpClient : public UObject
{
  GENERATED_BODY()

public:
  FORCEINLINE FString GetAPIKey() const { return APIKey; }
  FORCEINLINE FString GetAPIEndpoint() const { return APIEndpoint; }

  void ConfigureClient(FString InAPIKey, FString InEndpointOverride = "");

  void MakeAPIRequest(
    const FString &Endpoint,
    const FSkyboxAiHttpHeaders &Headers,
    const FString &Body,
    TFunction<void(const FString &)> Callback
    );

  template <typename T> static FString *SerializeJSON(const T &Object);
  template <typename T> static FString *SerializeJSON(const TArray<T> &Object);
  template <typename T> static T *DeserializeJSONToUStruct(const FString &Body);
  template <typename T> static TArray<T> DeserializeJSONToUStructArray(const FString &Body);

private:
  FString APIKey;
  FString APIEndpoint = TEXT("https://backend.blockadelabs.com/api/v1");

  void ShowHTTPError(const int32 Code, const FText &Message) const;
};

template <typename T> FString *USKyboxAiHttpClient::SerializeJSON(const T &Object)
{
  FString OutputString;

  if (!FJsonObjectConverter::UStructToJsonObjectString(T::StaticStruct(), &Object, OutputString, 0, 0))
  {
    FMessageLog(SkyboxAiHttpClient::GMessageLogName)
      .Error(FText::FromString(TEXT("Serialization from JSON failed")))
      ->AddToken(FTextToken::Create(FText::FromString(Object.ToString())));
    return nullptr;
  }

  return &OutputString;
}

template <typename T> FString *USKyboxAiHttpClient::SerializeJSON(const TArray<T> &Object)
{
  FString OutputString;

  if (!FJsonObjectConverter::UStructToJsonObjectString(T::StaticStruct(), &Object, OutputString, 0, 0))
  {
    FMessageLog(SkyboxAiHttpClient::GMessageLogName)
      .Error(FText::FromString(TEXT("Serialization from JSON failed")))
      ->AddToken(FTextToken::Create(FText::FromString(Object.ToString())));
    return nullptr;
  }

  return &OutputString;
}

template <typename T> T *USKyboxAiHttpClient::DeserializeJSONToUStruct(const FString &Body)
{
  T OutObject;

  if (!FJsonObjectConverter::JsonObjectStringToUStruct(Body, &OutObject, 0, 0))
  {
    FMessageLog(SkyboxAiHttpClient::GMessageLogName)
      .Error(FText::FromString(TEXT("Deserialization to JSON failed")))
      ->AddToken(FTextToken::Create(FText::FromString(Body)));
    return nullptr;
  }

  return &OutObject;
}

template <typename T> TArray<T> USKyboxAiHttpClient::DeserializeJSONToUStructArray(const FString &Body)
{
  TArray<T> OutObject;

  if (!FJsonObjectConverter::JsonArrayStringToUStruct(Body, &OutObject, 0, 0))
  {
    FMessageLog(SkyboxAiHttpClient::GMessageLogName)
      .Error(FText::FromString(TEXT("Deserialization to JSON failed")))
      ->AddToken(FTextToken::Create(FText::FromString(Body)));
    return TArray<T>();
  }

  return OutObject;
}
