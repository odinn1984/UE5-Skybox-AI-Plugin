#include "SKyboxAiHttpClient.h"
#include "BlockadeLabsSkyboxAiSettings.h"
#include "HttpModule.h"
#include "SkyboxAiApi.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"

DEFINE_LOG_CATEGORY(SkyboxAiHttpClient);

USKyboxAiHttpClient::USKyboxAiHttpClient()
{
  Http = &FHttpModule::Get();
}

void USKyboxAiHttpClient::SetHttpModule(FHttpModule *InHttp)
{
  Http = InHttp;
}

void USKyboxAiHttpClient::MakeAPIRequest(
  const FString &Endpoint,
  const FSkyboxAiHttpHeaders &Headers,
  const FString &Body,
  FSkyboxAiHttpCallback Callback) const
{
  const UBlockadeLabsSkyboxAiSettings *EditorSettings = GetMutableDefault<UBlockadeLabsSkyboxAiSettings>();
  const FString ApiKey = EditorSettings->ApiKey;
  const FString ApiEndpoint = EditorSettings->ApiEndpoint;

  const TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = Http->CreateRequest();
  const FString URL = ApiEndpoint + Endpoint;

  Request->SetURL(URL);
  Request->SetVerb(Headers.Method);
  Request->SetHeader("x-api-key", ApiKey);
  Request->SetHeader("accept", Headers.Accept);
  Request->SetHeader("Content-Type", Headers.ContentType);

  if (!Body.Equals(TEXT(""))) Request->SetContentAsString(Body);

  Request->ProcessRequest();
  Request->OnProcessRequestComplete().BindLambda(
    [this, Callback](FHttpRequestPtr Req, FHttpResponsePtr Res, bool bConnectedSuccessfully)
    {
      FString ResBody;
      int StatusCode = 0;

      if (!bConnectedSuccessfully || !Res.IsValid())
      {
        ResBody = TEXT("Failed to connect to the server");
        FMessageLog(SkyboxAiHttpClientDefinitions::GMessageLogName).
          Error(FText::FromString(ResBody))->
          AddToken(FTextToken::Create(FText::FromString(Req->GetURL())));
      }
      else
      {
        ResBody = Res.Get()->GetContentAsString();
        StatusCode = Res.Get()->GetResponseCode();

        if (StatusCode >= 300) HandleHttpResponse(Res.ToSharedRef());
      }

      Callback(ResBody, StatusCode, bConnectedSuccessfully);
    }
    );
}

void USKyboxAiHttpClient::HandleHttpResponse(FHttpResponseRef Response) const
{
  const int StatusCode = Response->GetResponseCode();

  if (StatusCode < 300) return;

  EMessageSeverity::Type Severity = EMessageSeverity::Warning;
  FString ErrorMessage = TEXT("Unexpected error occurred");
  FSkyboxApiError *ParsedResponse = DeserializeJsonToUStruct<FSkyboxApiError>(Response->GetContentAsString());

  if (ParsedResponse != nullptr) ErrorMessage = ParsedResponse->error;
  if (StatusCode >= 500) Severity = EMessageSeverity::Error;

  FMessageLog(SkyboxAiHttpClientDefinitions::GMessageLogName)
    .Message(Severity, FText::FromString(TEXT("HTTP Request Done")))
    ->AddToken(FTextToken::Create(FText::FromString(Response->GetURL())))
    ->AddToken(FTextToken::Create(FText::FromString(FString::SanitizeFloat(StatusCode))))
    ->AddToken(FTextToken::Create(FText::FromString(ErrorMessage)));
}
