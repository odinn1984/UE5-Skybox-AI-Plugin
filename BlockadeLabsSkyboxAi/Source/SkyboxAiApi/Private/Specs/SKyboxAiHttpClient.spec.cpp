#include "HttpModule.h"
#include "SkyboxAiApi.h"
#include "SKyboxAiHttpClient.h"

BEGIN_DEFINE_SPEC(
  FSkyboxAiHttpClientSpec,
  "BlockadeLabs.SkyboxAiHttpClient.Unit",
  EAutomationTestFlags::ProductFilter | EAutomationTestFlags::ApplicationContextMask
)
  FHttpModule* HttpModule;
  TObjectPtr<USKyboxAiHttpClient> HttpClient;

  static FString CleanJson(const FString& InJson)
  {
    return InJson
           .TrimStartAndEnd()
           .Replace(TEXT("\r"), TEXT(""))
           .Replace(TEXT("\n"), TEXT(""))
           .Replace(TEXT(" "), TEXT(""))
           .Replace(TEXT("\t"),TEXT(""));
  }

END_DEFINE_SPEC(FSkyboxAiHttpClientSpec)

void FSkyboxAiHttpClientSpec::Define()
{
  BeforeEach(
    [this]()
    {
      HttpClient = NewObject<USKyboxAiHttpClient>();
      TestNotNull("HttpClient should not be null", HttpClient.Get());

      HttpModule = &FHttpModule::Get();
      TestNotNull("HttpModule should not be null", HttpModule);

      HttpModule->ToggleNullHttp(true);
      HttpClient->SetHttpModule(HttpModule);
      TestEqual("HTTP module was updated", HttpClient->HttpModule(), HttpModule);
    }
  );

  Describe(
    "SkyboxAiHttpClient",
    [this]()
    {
      Describe(
        "SerializeJson",
        [this]()
        {
          It(
            "Should serialize object to string",
            EAsyncExecution::ThreadPool,
            [this]()
            {
              FSkyboxAiHttpHeaders JsonInput = FSkyboxAiHttpHeaders();
              FString OutString;
              bool bSuccess = HttpClient->SerializeJson<FSkyboxAiHttpHeaders>(JsonInput, OutString);

              TestTrue("Call should succeed", bSuccess);
              TestTrue("OutString should not be empty", !OutString.IsEmpty());
              TestEqual(
                "OutString should be equal to JsonInput",
                CleanJson(OutString),
                "{\"method\":\"GET\",\"accept\":\"application/json\",\"contentType\":\"application/json\"}"
              );
            }
          );

          It(
            "Should serialize array to string",
            EAsyncExecution::ThreadPool,
            [this]()
            {
              TArray<FSkyboxAiHttpHeaders> JsonInput;
              JsonInput.Add(FSkyboxAiHttpHeaders());
              JsonInput.Add(FSkyboxAiHttpHeaders());

              FString OutString;
              bool bSuccess = HttpClient->SerializeJson<FSkyboxAiHttpHeaders>(JsonInput, OutString);

              TestTrue("Call should succeed", bSuccess);
              TestTrue("OutString should not be empty", !OutString.IsEmpty());
              TestEqual(
                "OutString should be equal to JsonInput",
                CleanJson(OutString),
                "[{\"method\":\"GET\",\"accept\":\"application/json\",\"contentType\":\"application/json\"},{\"method\":\"GET\",\"accept\":\"application/json\",\"contentType\":\"application/json\"}]"
              );
            }
          );
        }
      );

      Describe(
        "DeserializeJsonToUStruct",
        [this]()
        {
          It(
            "Should deserialize to struct",
            EAsyncExecution::ThreadPool,
            [this]()
            {
              FString ErrorMessage = "{\"error\":\"Invalid API key\"}";
              FSkyboxApiError ParsedResponse;

              bool bSuccess = HttpClient->DeserializeJsonToUStruct<FSkyboxApiError>(ErrorMessage, &ParsedResponse);

              TestTrue("Call should succeed", bSuccess);
              TestEqual("Error message should be equal", ParsedResponse.error, "Invalid API key");
            }
          );

          It(
            "Should return false if deserialize to struct failed",
            EAsyncExecution::ThreadPool,
            [this]()
            {
              AddExpectedMessage(TEXT("JsonObjectStringToUStruct"), EAutomationExpectedErrorFlags::Contains, 1);
              AddExpectedError(TEXT("Deserialization to JSON failed"), EAutomationExpectedErrorFlags::Contains, 1);

              FString ErrorMessage = "{\"error\":\"Invalid API key";
              FSkyboxApiError ParsedResponse;

              bool bSuccess = HttpClient->DeserializeJsonToUStruct<FSkyboxApiError>(ErrorMessage, &ParsedResponse);

              TestFalse("Call should not succeed", bSuccess);
              TestEqual("Error message should be equal", ParsedResponse.error, "");
            }
          );
        }
      );

      Describe(
        "DeserializeJsonToUStructArray",
        [this]()
        {
          It(
            "Should deserialize to array",
            EAsyncExecution::ThreadPool,
            [this]()
            {
              FString ErrorMessage = "[{\"error\":\"Invalid API key\"},{\"error\":\"Invalid API request\"}]";
              TArray<FSkyboxApiError> ParsedResponse;

              bool bSuccess = HttpClient->DeserializeJsonToUStructArray<FSkyboxApiError>(ErrorMessage, &ParsedResponse);

              TestTrue("Call should succeed", bSuccess);
              TestEqual("Length should be 2", ParsedResponse.Num(), 2);
              TestEqual("Error message 1 should be equal", ParsedResponse[0].error, "Invalid API key");
              TestEqual("Error message 2 should be equal", ParsedResponse[1].error, "Invalid API request");
            }
          );

          It(
            "Should return false if deserialize json array to struct failed",
            EAsyncExecution::ThreadPool,
            [this]()
            {
              AddExpectedMessage(TEXT("JsonArrayStringToUStruct"), EAutomationExpectedErrorFlags::Contains, 1);
              AddExpectedError(TEXT("Deserialization to JSON Array failed"), EAutomationExpectedErrorFlags::Contains, 1);

              FString ErrorMessage = "[{\"error\":\"Invalid API key\"},{\"error\":\"Invalid API request\"}";
              TArray<FSkyboxApiError> ParsedResponse;

              bool bSuccess = HttpClient->DeserializeJsonToUStructArray<FSkyboxApiError>(ErrorMessage, &ParsedResponse);

              TestFalse("Call should not succeed", bSuccess);
              TestEqual("Length should be 0", ParsedResponse.Num(), 0);
            }
          );
        }
      );
    }
  );
}
