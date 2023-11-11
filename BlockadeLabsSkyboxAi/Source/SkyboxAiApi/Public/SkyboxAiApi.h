﻿#pragma once

#include "CoreMinimal.h"
#include "SkyboxAiApi.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(SkyboxAiApi, Log, All);

class USkyboxProvider;
class UImagineProvider;
class USKyboxAiHttpClient;

struct FImagineGetExportsResponse;

USTRUCT(BlueprintType)
struct FSkyboxApiError
{
  GENERATED_BODY()

  UPROPERTY()
  FString error = TEXT("");
};

UCLASS()
class SKYBOXAIAPI_API USkyboxApi : public UObject, public IModuleInterface
{
  GENERATED_BODY()

public:
  explicit USkyboxApi();

  FORCEINLINE USkyboxProvider *Skybox() const { return SkyboxProvider; }
  FORCEINLINE UImagineProvider *Imagine() const { return ImagineProvider; }

  FORCEINLINE void SetClient(USKyboxAiHttpClient *InAPIClient) { ApiClient = InAPIClient; }

  void SaveExportedImage(const FString &ImageUrl, TFunction<void(bool bSuccess)> Callback) const;

protected:
  virtual bool IsClientValid() const;

private:
  UPROPERTY()
  TObjectPtr<USKyboxAiHttpClient> ApiClient;

  UPROPERTY()
  TObjectPtr<USkyboxProvider> SkyboxProvider;

  UPROPERTY()
  TObjectPtr<UImagineProvider> ImagineProvider;

  void DownloadImage(
    const FString &ImageUrl,
    TFunction<void(TArray<uint8> ExportedImage, bool bSuccess)> Callback) const;
};
