#pragma once

#include "CoreMinimal.h"
#include "SKyboxAiHttpClient.h"
#include "SkyboxProvider.generated.h"

class USKyboxAiHttpClient;

struct FSkyboxGenerateResponse;
struct FSkyboxExportResponse;

struct FSkyboxListEntry
{
  FString Name;
  FString ImageUrl;
  int PromptMaxLen;
  int NegativeTextMaxLen;

  bool operator==(const FSkyboxListEntry &Other) const
  {
    return Name.Equals(Other.Name);
  }

  bool operator==(FString &OtherName) const
  {
    return Name.Equals(OtherName);
  }
};

typedef TMap<int, FSkyboxListEntry> FSkyboxAiStyles;
typedef TMap<int, FString> FSkyboxAiExportTypes;

typedef TFunction<void(FSkyboxGenerateResponse *, int StatusCode, bool bConnectedSuccessfully)> FPostCallback;
typedef TFunction<void(FSkyboxAiStyles &, int StatusCode, bool bConnectedSuccessfully)> FGetStylesCallback;
typedef TFunction<void(FSkyboxAiExportTypes &, int StatusCode, bool bConnectedSuccessfully)> FGetExportsCallback;
typedef TFunction<void(FSkyboxExportResponse *, int StatusCode, bool bConnectedSuccessfully)> FGetExportCallback;
typedef TFunction<void(FSkyboxExportResponse *, int StatusCode, bool bConnectedSuccessfully)> FPostExportCallback;

USTRUCT()
struct FSkyboxExportRequest
{
  GENERATED_BODY()

  UPROPERTY()
  FString skybox_id = TEXT("");

  UPROPERTY()
  int type_id = 0;
};

USTRUCT(BlueprintType)
struct FSkyboxStyle
{
  GENERATED_BODY()

  UPROPERTY()
  int id = 0;

  UPROPERTY()
  FString name = TEXT("");

  UPROPERTY()
  int premium = 0;

  UPROPERTY()
  int max_char = 0;

  UPROPERTY()
  int negative_text_max_char = 0;

  UPROPERTY()
  FString image = TEXT("");
};

USTRUCT()
struct FSkyboxExportType
{
  GENERATED_BODY()

  UPROPERTY()
  int id = 0;

  UPROPERTY()
  FString name = TEXT("");

  UPROPERTY()
  bool premium_feature = false;
};

USTRUCT()
struct FSkyboxExportResponse
{
  GENERATED_BODY()

  UPROPERTY()
  FString file_url = TEXT("");

  UPROPERTY()
  FString id = TEXT("");

  UPROPERTY()
  FString status = TEXT("");

  UPROPERTY()
  FString error_message = TEXT("");
};

USTRUCT()
struct FSkyboxGenerateResponse
{
  GENERATED_BODY()

  UPROPERTY()
  FString obfuscated_id = TEXT("");

  UPROPERTY()
  FString status = TEXT("");

  UPROPERTY()
  FString error_message = TEXT("");
};

USTRUCT()
struct FSkyboxGenerateRequest
{
  GENERATED_BODY()

  UPROPERTY()
  FString prompt = TEXT("");

  UPROPERTY()
  FString negative_text = SkyboxAiHttpClientDefinitions::GIgnoreStringJsonFieldValue;

  UPROPERTY()
  bool enhance_prompt = false;

  UPROPERTY()
  int skybox_style_id = INDEX_NONE;

  UPROPERTY()
  int remix_imagine_id = INDEX_NONE;
};

UCLASS()
class SKYBOXAIAPI_API USkyboxProvider : public UObject
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
  TObjectPtr<USKyboxAiHttpClient> ApiClient;

  virtual bool IsClientValid() const;
  virtual bool ShouldShowPremiumContent() const;
  virtual bool RemovePostUnsetFields(const FSkyboxGenerateRequest &Data, FString *OutBody) const;
  virtual FString GetStylesResponseKeysHyphensToUnderscore(const FString &Body) const;
};
