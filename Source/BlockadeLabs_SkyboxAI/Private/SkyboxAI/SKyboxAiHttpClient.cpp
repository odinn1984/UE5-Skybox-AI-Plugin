#include "SKyboxAiHttpClient.h"
#include "BlockadeLabs_SkyboxAISettings.h"
#include "SkyboxProvider.h"
#include "HttpModule.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"

DEFINE_LOG_CATEGORY(SkyboxAIAPI);

USkyboxApi::USkyboxApi()
{
  UBlockadeLabs_SkyboxAISettings *EditorSettings = GetMutableDefault<UBlockadeLabs_SkyboxAISettings>();
  const FString APIKey = EditorSettings->APIKey;
  const FString APIEndpoint = EditorSettings->APIEndpoint;

  EditorSettings->OnSettingChanged().AddUObject(this, &USkyboxApi::OnSettingsChanged);

  APIClient = CreateDefaultSubobject<USKyboxAiHttpClient>(TEXT("SkyboxAiHttpClient"));
  APIClient->ConfigureClient(APIKey, APIEndpoint);

  SkyboxProvider = CreateDefaultSubobject<USkyboxProvider>(TEXT("SkyboxProvider"));
  SkyboxProvider->SetClient(APIClient);
}

void USkyboxApi::OnSettingsChanged(UObject *Object, FPropertyChangedEvent &Event)
{
  APIClient->ConfigureClient(
    CastChecked<UBlockadeLabs_SkyboxAISettings>(Object)->APIKey,
    CastChecked<UBlockadeLabs_SkyboxAISettings>(Object)->APIEndpoint
    );
}

void USKyboxAiHttpClient::ConfigureClient(FString InAPIKey, FString InEndpointOverride)
{
  APIKey = InAPIKey;
  if (!InEndpointOverride.Equals(TEXT(""))) APIEndpoint = InEndpointOverride;
}

void USKyboxAiHttpClient::MakeAPIRequest(
  const FString &Endpoint,
  const FSkyboxAiHttpHeaders &Headers,
  const FString &Body,
  TFunction<void(const FString &)> Callback)
{
  FHttpModule *Http = &FHttpModule::Get();
  const TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = Http->CreateRequest();

  const FString URL = APIEndpoint + Endpoint;

  Request->SetURL(URL);
  Request->SetVerb(Headers.Method);
  Request->SetHeader("x-api-key", APIKey);
  Request->SetHeader("accept", Headers.Accept);
  Request->SetHeader("Content-Type", Headers.ContentType);

  if (!Body.Equals(TEXT(""))) Request->SetContentAsString(Body);

  FMessageLog(SkyboxAiHttpClient::GMessageLogName)
    .Info(FText::FromString(TEXT("Starting Request")))
    ->AddToken(FTextToken::Create(FText::FromString(Body)));

  Request->ProcessRequest();
  Request->OnProcessRequestComplete().BindLambda(
    [this, Callback](FHttpRequestPtr Req, FHttpResponsePtr Res, bool bWasSuccessful)
    {
      FString ResBody = TEXT("No Data");

      if (!bWasSuccessful || !Res.IsValid())
      {
        const int32 StatusCode = Res.Get()->GetResponseCode();
        if (Res.IsValid()) ResBody = Res.Get()->GetContentAsString();

        ShowHTTPError(StatusCode, FText::FromString(TEXT("Failed Making Request to Skybox AI")));

        const FString ErrMsg = FString::Printf(TEXT("Failed Request Status Code (%d) ---- %s"), StatusCode, *ResBody);
        FMessageLog(SkyboxAiHttpClient::GMessageLogName).Error(FText::FromString(ErrMsg));

        return;
      }

      ResBody = Res.Get()->GetContentAsString();

      Callback(ResBody);
      FMessageLog(SkyboxAiHttpClient::GMessageLogName)
        .Info(FText::FromString(TEXT("Finished Request")))
        ->AddToken(FTextToken::Create(FText::FromString(ResBody)));
    }
    );
}

// ReSharper disable once CppMemberFunctionMayBeStatic
void USKyboxAiHttpClient::ShowHTTPError(const int32 Code, const FText &Message) const
{
  FNotificationInfo Info(Message);

  Info.bFireAndForget = true;
  Info.FadeOutDuration = 3.0f;
  Info.ExpireDuration = 10.0f;
  Info.Hyperlink = FSimpleDelegate::CreateLambda(
    []()
    {
      FGlobalTabmanager::Get()->TryInvokeTab(FName("MessageLog"));
    }
    );
  Info.HyperlinkText = FText::FromString(TEXT("View Message Log"));

  const TSharedPtr<SNotificationItem> NotificationItem = FSlateNotificationManager::Get().AddNotification(Info);

  if (NotificationItem.IsValid())
  {
    NotificationItem->SetCompletionState(SNotificationItem::CS_Fail);
    NotificationItem->ExpireAndFadeout();
  }
}
