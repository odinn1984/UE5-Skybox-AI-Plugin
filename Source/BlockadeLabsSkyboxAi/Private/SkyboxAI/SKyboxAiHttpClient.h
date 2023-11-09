#pragma once

#include "CoreMinimal.h"
#include "HttpFwd.h"
#include "JsonObjectConverter.h"
#include "SkyboxAiHttpClient.generated.h"

struct FImagineGetExportsResponse;
class FHttpModule;
typedef TFunction<void(const FString &Body, int StatusCode, bool bConnectedSuccessfully)> FSkyboxAiHttpCallback;

class USkyboxProvider;
class UImagineProvider;
class USKyboxAiHttpClient;

DECLARE_LOG_CATEGORY_EXTERN(SkyboxAiAPI, Log, All);

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

USTRUCT(BlueprintType)
struct FSkyboxApiError
{
  GENERATED_BODY()

  UPROPERTY()
  FString error;
};

UCLASS()
class USkyboxApi : public UObject
{
  GENERATED_BODY()

public:
  explicit USkyboxApi();

  FORCEINLINE USkyboxProvider *Skybox() const { return SkyboxProvider; }
  FORCEINLINE UImagineProvider *Imagine() const { return ImagineProvider; }

  void SaveExportedImage(const FString &Id) const;

protected:
  virtual bool IsClientValid() const;

private:
  UPROPERTY()
  TObjectPtr<USKyboxAiHttpClient> ApiClient;

  UPROPERTY()
  TObjectPtr<USkyboxProvider> SkyboxProvider;

  UPROPERTY()
  TObjectPtr<UImagineProvider> ImagineProvider;

  bool ValidateExportedImageCall(
    FImagineGetExportsResponse *Response,
    int StatusCode,
    bool bConnectedSuccessfully) const;
};

UCLASS()
class USKyboxAiHttpClient : public UObject
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
    FMessageLog(SkyboxAiHttpClient::GMessageLogName).Error(FText::FromString(TEXT("Serialization from JSON failed")));
    return nullptr;
  }

  return &OutputString;
}

template <typename T> FString *USKyboxAiHttpClient::SerializeJson(const TArray<T> &Object)
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

template <typename T> T *USKyboxAiHttpClient::DeserializeJsonToUStruct(const FString &Body)
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

template <typename T> TArray<T> USKyboxAiHttpClient::DeserializeJsonToUStructArray(const FString &Body)
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
