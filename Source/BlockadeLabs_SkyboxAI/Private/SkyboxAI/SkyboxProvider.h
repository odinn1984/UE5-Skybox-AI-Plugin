#pragma once

#include "CoreMinimal.h"
#include "SkyboxProvider.generated.h"

class USKyboxAiHttpClient;

struct FSkyboxGenerateResponse;
struct FSkyboxExportResponse;

typedef const TMap<int, FString> FSkyboxAiStyles;
typedef const TMap<int, FString> FSkyboxAiExportTypes;

typedef TFunction<void(int StatusCode, bool bConnectedSuccessfully)> FPostCallback;
typedef TFunction<void(FSkyboxAiStyles &, int StatusCode, bool bConnectedSuccessfully)> FGetStylesCallback;
typedef TFunction<void(FSkyboxAiExportTypes &, int StatusCode, bool bConnectedSuccessfully)> FGetExportsCallback;
typedef TFunction<void(FSkyboxExportResponse *, int StatusCode, bool bConnectedSuccessfully)> FGetExportCallback;
typedef TFunction<void(int StatusCode, bool bConnectedSuccessfully)> FPostExportCallback;

USTRUCT()
struct FSkyboxExportRequest
{
  GENERATED_BODY()
};

USTRUCT(BlueprintType)
struct FSkyboxStyle
{
  GENERATED_BODY()

  UPROPERTY()
  int id;

  UPROPERTY()
  FString name;
};

USTRUCT()
struct FSkyboxExportType
{
  GENERATED_BODY()

  UPROPERTY()
  int id;

  UPROPERTY()
  FString name;
};

USTRUCT()
struct FSkyboxExportResponse
{
  GENERATED_BODY()

  UPROPERTY()
  FString file_url;

  UPROPERTY()
  FString id;

  UPROPERTY()
  FString status;

  UPROPERTY()
  FString error_message;
};

USTRUCT()
struct FSkyboxGenerateResponse
{
  GENERATED_BODY()
};

USTRUCT()
struct FSkyboxGenerateRequest
{
  GENERATED_BODY()
};

UCLASS()
class USkyboxProvider : public UObject
{
  GENERATED_BODY()

public:
  explicit USkyboxProvider();

  void SetClient(USKyboxAiHttpClient *InAPIClient);

  virtual void Post(const FSkyboxGenerateRequest &Data, FPostCallback Callback) const;

  virtual void GetStyles(FGetStylesCallback Callback) const;
  virtual void GetExports(FGetExportsCallback Callback) const;

  virtual void GetExport(const FString Id, FGetExportCallback Callback) const;
  virtual void PostExport(const FSkyboxExportRequest &Data, FPostExportCallback Callback) const;

protected:
  UPROPERTY()
  TObjectPtr<USKyboxAiHttpClient> APIClient;
};
