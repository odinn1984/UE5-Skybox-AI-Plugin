#pragma once

#include "CoreMinimal.h"

#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 2
#include "HttpFwd.h"
#elif ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION <= 1
#include "Interfaces/IHttpRequest.h"
#endif

#include "JsonObjectConverter.h"
#include "Logging/MessageLog.h"
#include "SkyboxAiHttpClient.generated.h"

typedef TFunction<void(const FString& Body, int StatusCode, bool bConnectedSuccessfully)> FSkyboxAiHttpCallback;

DECLARE_LOG_CATEGORY_EXTERN(SkyboxAiHttpClient, Log, All);

class FHttpModule;

namespace SkyboxAiHttpClientDefinitions
{
  const static FName GMessageLogName = TEXT("SkyboxAI");
  const static FString GIgnoreStringJsonFieldValue = TEXT("IGNOREME");
  const static FString JsonFailureString = TEXT("--FAILURE--");

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

  FORCEINLINE FHttpModule* HttpModule() const { return Http; }

  virtual void SetHttpModule(FHttpModule* InHttp);
  virtual void MakeAPIRequest(
    const FString& Endpoint,
    const FSkyboxAiHttpHeaders& Headers,
    const FString& Body,
    FSkyboxAiHttpCallback Callback);

  template <typename T> bool SerializeJson(const T& Object, FString& OutString);
  template <typename T> bool SerializeJson(const TArray<T>& ObjectArr, FString& OutString);
  template <typename T> bool DeserializeJsonToUStruct(const FString& Body, T* OutObject);
  template <typename T> bool DeserializeJsonToUStructArray(const FString& Body, TArray<T>* OutArray);

private:
  FHttpModule* Http;

  void HandleHttpResponse(FHttpResponseRef Res);
};

template <typename T> bool USKyboxAiHttpClient::SerializeJson(const T& Object, FString& OutString)
{
  if (!FJsonObjectConverter::UStructToJsonObjectString(T::StaticStruct(), &Object, OutString, 0, 0))
  {
    FMessageLog(SkyboxAiHttpClientDefinitions::GMessageLogName).Error(FText::FromString(TEXT("Serialization from JSON failed")));
    OutString.Empty();
    return false;
  }

  return true;
}

template <typename T> bool USKyboxAiHttpClient::SerializeJson(const TArray<T>& ObjectArr, FString& OutString)
{
  TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutString);
  Writer->WriteArrayStart();

  for (const T& Item : ObjectArr)
  {
    TSharedPtr<FJsonObject> JsonObject = FJsonObjectConverter::UStructToJsonObject(Item);

    if (!JsonObject.IsValid())
    {
      FMessageLog(SkyboxAiHttpClientDefinitions::GMessageLogName).Error(FText::FromString(TEXT("Serialization from JSON failed")));
      Writer->Close();
      OutString.Empty();
      return false;
    }

    FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);
  }

  Writer->WriteArrayEnd();

  return Writer->Close();
}

template <typename T> bool USKyboxAiHttpClient::DeserializeJsonToUStruct(const FString& Body, T* OutObject)
{
  if (!FJsonObjectConverter::JsonObjectStringToUStruct(Body, OutObject, 0, 0))
  {
    FMessageLog(SkyboxAiHttpClientDefinitions::GMessageLogName)
      .Error(FText::FromString(TEXT("Deserialization to JSON failed")))
      ->AddToken(FTextToken::Create(FText::FromString(Body)));
    return false;
  }

  return true;
}

template <typename T> bool USKyboxAiHttpClient::DeserializeJsonToUStructArray(const FString& Body, TArray<T>* OutArray)
{
  if (!FJsonObjectConverter::JsonArrayStringToUStruct(Body, OutArray, 0, 0))
  {
    if (OutArray != nullptr) OutArray->Empty();
    FMessageLog(SkyboxAiHttpClientDefinitions::GMessageLogName)
      .Error(FText::FromString(TEXT("Deserialization to JSON Array failed")))
      ->AddToken(FTextToken::Create(FText::FromString(Body)));
    return false;
  }

  return true;
}
