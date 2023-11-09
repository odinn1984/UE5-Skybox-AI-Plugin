#pragma once

#include "CoreMinimal.h"
#include "HttpFwd.h"
#include "JsonObjectConverter.h"
#include "SkyboxAiHttpClient.generated.h"

typedef TFunction<void(const FString &Body, int StatusCode, bool bConnectedSuccessfully)> FSkyboxAiHttpCallback;

DECLARE_LOG_CATEGORY_EXTERN(SkyboxAiHttpClient, Log, All);

class FHttpModule;

namespace SkyboxAiHttpClientDefinitions
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
  FString Method = SkyboxAiHttpClientDefinitions::HTTPVerbs::GGet;

  UPROPERTY()
  FString Accept = SkyboxAiHttpClientDefinitions::ContentTypes::GJson;

  UPROPERTY()
  FString ContentType = SkyboxAiHttpClientDefinitions::ContentTypes::GJson;
};

UCLASS()
class SKYBOXAIAPI_API USKyboxAiHttpClient : public UObject
{
  GENERATED_BODY()

public:
  USKyboxAiHttpClient();

  void SetHttpModule(FHttpModule *InHttp);

  void MakeAPIRequest(
    const FString &Endpoint,
    const FSkyboxAiHttpHeaders &Headers,
    const FString &Body,
    FSkyboxAiHttpCallback Callback) const;

  template <typename T> static FString *SerializeJson(const T &Object);
  template <typename T> static FString *SerializeJson(const TArray<T> &Object);
  template <typename T> static T *DeserializeJsonToUStruct(const FString &Body);
  template <typename T> static TArray<T> DeserializeJsonToUStructArray(const FString &Body);

private:
  FHttpModule *Http;

  void HandleHttpResponse(FHttpResponseRef Res) const;
};

template <typename T> FString *USKyboxAiHttpClient::SerializeJson(const T &Object)
{
  FString OutputString;

  if (!FJsonObjectConverter::UStructToJsonObjectString(T::StaticStruct(), &Object, OutputString, 0, 0))
  {
    FMessageLog(SkyboxAiHttpClientDefinitions::GMessageLogName).Error(FText::FromString(TEXT("Serialization from JSON failed")));
    return nullptr;
  }

  return &OutputString;
}

template <typename T> FString *USKyboxAiHttpClient::SerializeJson(const TArray<T> &Object)
{
  FString OutputString;

  if (!FJsonObjectConverter::UStructToJsonObjectString(T::StaticStruct(), &Object, OutputString, 0, 0))
  {
    FMessageLog(SkyboxAiHttpClientDefinitions::GMessageLogName)
      .Error(FText::FromString(TEXT("Serialization from JSON failed")))
      ->AddToken(FTextToken::Create(FText::FromString(Object.ToString())));
    return nullptr;
  }

  return &OutputString;
}

template <typename T> T *USKyboxAiHttpClient::DeserializeJsonToUStruct(const FString &Body)
{
  T OutObject;

  if (!FJsonObjectConverter::JsonObjectStringToUStruct(Body, &OutObject, 0, 0))
  {
    FMessageLog(SkyboxAiHttpClientDefinitions::GMessageLogName)
      .Error(FText::FromString(TEXT("Deserialization to JSON failed")))
      ->AddToken(FTextToken::Create(FText::FromString(Body)));
    return nullptr;
  }

  return &OutObject;
}

template <typename T> TArray<T> USKyboxAiHttpClient::DeserializeJsonToUStructArray(const FString &Body)
{
  TArray<T> OutObject;

  if (!FJsonObjectConverter::JsonArrayStringToUStruct(Body, &OutObject, 0, 0))
  {
    FMessageLog(SkyboxAiHttpClientDefinitions::GMessageLogName)
      .Error(FText::FromString(TEXT("Deserialization to JSON failed")))
      ->AddToken(FTextToken::Create(FText::FromString(Body)));
    return TArray<T>();
  }

  return OutObject;
}
