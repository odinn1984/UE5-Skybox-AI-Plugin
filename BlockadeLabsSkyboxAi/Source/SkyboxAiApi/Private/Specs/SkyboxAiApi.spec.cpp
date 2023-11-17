#include "BlockadeLabsSkyboxAiSettings.h"
#include "ImagineProvider.h"
#include "SKyboxAiHttpClient.h"
#include "SkyboxAiApi.h"
#include "SkyboxProvider.h"
#include "Mocks/MockSkyboxAiHttpClient.h"

BEGIN_DEFINE_SPEC(FSkyboxAiApiSpec, "BlockadeLabs.SkyboxAiApi.Unit", EAutomationTestFlags::ProductFilter | EAutomationTestFlags::ApplicationContextMask)
	TObjectPtr<USkyboxAiApi> ApiInstance;
	TObjectPtr<UMockSkyboxAiHttpClient> HttpClient;
END_DEFINE_SPEC(FSkyboxAiApiSpec)

void FSkyboxAiApiSpec::Define()
{
	BeforeEach(
		[this]()
		{
			HttpClient = NewObject<UMockSkyboxAiHttpClient>();
			TestNotNull("HttpClient should not be null", HttpClient.Get());

			ApiInstance = NewObject<USkyboxAiApi>();
			TestNotNull("SkyboxAiApi should not be null", ApiInstance.Get());
			TestNotNull("SkyboxProvider should not be null", ApiInstance->Skybox());
			TestNotNull("ImagineProvider should not be null", ApiInstance->Imagine());

			ApiInstance->SetClient(HttpClient);
			TestTrue("API Client was updated", ApiInstance->Client()->IsA(UMockSkyboxAiHttpClient::StaticClass()));
			TestTrue("Skybox API Client was updated", ApiInstance->Skybox()->Client()->IsA(UMockSkyboxAiHttpClient::StaticClass()));
			TestTrue("Imagine API Client was updated", ApiInstance->Imagine()->Client()->IsA(UMockSkyboxAiHttpClient::StaticClass()));
		}
	);

	Describe(
		"SkyboxAiApi",
		[this]()
		{
			Describe(
				"SkyboxProvider",
				[this]
				{
					Describe(
						"Post",
						[this]()
						{
							LatentIt(
								"Should be able to get a list of requests",
								EAsyncExecution::ThreadPool,
								[this](const FDoneDelegate& Done)
								{
									FSkyboxGenerateRequest Data;
									ApiInstance.Get()->Skybox()->Post(
										Data,
										[this, Done](
										FSkyboxGenerateResponse* Response,
										int StatusCode,
										bool bConnectedSuccessfully)
										{
											TestNotNull("Response should not be null", Response);
											TestEqual("StatusCode should be 200", StatusCode, 200);
											TestTrue("bConnectedSuccessfully should be true", bConnectedSuccessfully);
											TestEqual("Correct obfuscated id", Response->obfuscated_id, TEXT("c60354672339614111cee85d21253244"));
											TestEqual("Correct status", Response->status, TEXT("pending"));
											Done.Execute();
										}
									);
								}
							);
						}
					);
					Describe(
						"GetStyles",
						[this]()
						{
							LatentIt(
								"Should be able to get a list of styles with premium",
								EAsyncExecution::ThreadPool,
								[this](const FDoneDelegate& Done)
								{
									UBlockadeLabsSkyboxAiSettings* EditorSettings = GetMutableDefault<UBlockadeLabsSkyboxAiSettings>();
									bool bOriginalEnablePremiumContent = EditorSettings->bEnablePremiumContent;
									EditorSettings->bEnablePremiumContent = true;

									ApiInstance.Get()->Skybox()->GetStyles(
										[this, EditorSettings, bOriginalEnablePremiumContent, Done](
										FSkyboxAiStyles& Response,
										int StatusCode,
										bool bConnectedSuccessfully)
										{
											TestEqual("StatusCode should be 200", StatusCode, 200);
											TestTrue("bConnectedSuccessfully should be true", bConnectedSuccessfully);
											TestEqual("Correct number of styles", Response.Num(), 2);
											TestEqual("Correct name", Response[5].Name, "Digital Painting");
											TestEqual("Correct prompt max chars", Response[5].PromptMaxLen, 383);
											TestEqual("Correct negative max chars", Response[5].NegativeTextMaxLen, 260);
											TestEqual("Correct name", Response[40].Name, "Epic Digital Painting");
											TestEqual("Correct prompt max chars", Response[40].PromptMaxLen, 305);
											TestEqual("Correct negative max chars", Response[40].NegativeTextMaxLen, 120);

											EditorSettings->bEnablePremiumContent = bOriginalEnablePremiumContent;

											Done.Execute();
										}
									);
								}
							);

							LatentIt(
								"Should be able to get a list of styles without premium",
								EAsyncExecution::ThreadPool,
								[this](const FDoneDelegate& Done)
								{
									UBlockadeLabsSkyboxAiSettings* EditorSettings = GetMutableDefault<UBlockadeLabsSkyboxAiSettings>();
									bool bOriginalEnablePremiumContent = EditorSettings->bEnablePremiumContent;
									EditorSettings->bEnablePremiumContent = false;

									ApiInstance.Get()->Skybox()->GetStyles(
										[this, EditorSettings, bOriginalEnablePremiumContent, Done](
										FSkyboxAiStyles& Response,
										int StatusCode,
										bool bConnectedSuccessfully)
										{
											TestEqual("StatusCode should be 200", StatusCode, 200);
											TestTrue("bConnectedSuccessfully should be true", bConnectedSuccessfully);
											TestEqual("Correct number of styles", Response.Num(), 1);
											TestEqual("Correct name", Response[5].Name, "Digital Painting");
											TestEqual("Correct prompt max chars", Response[5].PromptMaxLen, 383);
											TestEqual("Correct negative max chars", Response[5].NegativeTextMaxLen, 260);

											EditorSettings->bEnablePremiumContent = bOriginalEnablePremiumContent;

											Done.Execute();
										}
									);
								}
							);
						}
					);
					Describe(
						"GetExports",
						[this]()
						{
							LatentIt(
								"Should be able to get a list of export types with premium",
								EAsyncExecution::ThreadPool,
								[this](const FDoneDelegate& Done)
								{
									UBlockadeLabsSkyboxAiSettings* EditorSettings = GetMutableDefault<UBlockadeLabsSkyboxAiSettings>();
									bool bOriginalEnablePremiumContent = EditorSettings->bEnablePremiumContent;
									EditorSettings->bEnablePremiumContent = true;

									ApiInstance.Get()->Skybox()->GetExport(
										[this, EditorSettings, bOriginalEnablePremiumContent, Done](
										FSkyboxAiExportTypes& Response,
										int StatusCode,
										bool bConnectedSuccessfully)
										{
											TestEqual("StatusCode should be 200", StatusCode, 200);
											TestTrue("bConnectedSuccessfully should be true", bConnectedSuccessfully);
											TestEqual("Correct number of export types", Response.Num(), 9);
											TestEqual("Correct name", Response[1], "JPG");
											TestEqual("Correct name", Response[2], "PNG");
											TestEqual("Correct name", Response[3], "Cube Map");

											EditorSettings->bEnablePremiumContent = bOriginalEnablePremiumContent;

											Done.Execute();
										}
									);
								}
							);

							LatentIt(
								"Should be able to get a list of export types without premium",
								EAsyncExecution::ThreadPool,
								[this](const FDoneDelegate& Done)
								{
									UBlockadeLabsSkyboxAiSettings* EditorSettings = GetMutableDefault<UBlockadeLabsSkyboxAiSettings>();
									bool bOriginalEnablePremiumContent = EditorSettings->bEnablePremiumContent;
									EditorSettings->bEnablePremiumContent = false;

									ApiInstance.Get()->Skybox()->GetExport(
										[this, EditorSettings, bOriginalEnablePremiumContent, Done](
										FSkyboxAiExportTypes& Response,
										int StatusCode,
										bool bConnectedSuccessfully)
										{
											TestEqual("StatusCode should be 200", StatusCode, 200);
											TestTrue("bConnectedSuccessfully should be true", bConnectedSuccessfully);
											TestEqual("Correct number of export types", Response.Num(), 2);
											TestEqual("Correct name", Response[1], "JPG");
											TestEqual("Correct name", Response[2], "PNG");

											EditorSettings->bEnablePremiumContent = bOriginalEnablePremiumContent;

											Done.Execute();
										}
									);
								}
							);
						}
					);

					Describe(
						"GetExport",
						[this]()
						{
							LatentIt(
								"Should get information about an export",
								EAsyncExecution::ThreadPool,
								[this](const FDoneDelegate& Done)
								{
									ApiInstance.Get()->Skybox()->GetExportById(
										TEXT("b26584eb869cdebeecdf009b7c17c73b"),
										[this, Done](
										FSkyboxExportResponse* Response,
										int StatusCode,
										bool bConnectedSuccessfully)
										{
											TestNotNull("Response should not be null", Response);
											TestEqual("StatusCode should be 200", StatusCode, 200);
											TestTrue("bConnectedSuccessfully should be true", bConnectedSuccessfully);
											TestEqual("Correct obfuscated id", Response->id, TEXT("b26584eb869cdebeecdf009b7c17c73b"));
											TestEqual("Correct status", Response->status, TEXT("complete"));
											TestEqual(
												"Correct file URL",
												Response->file_url,
												TEXT(
													"https://blockade-platform-production.s3.amazonaws.com/exports/equirectangular-png/vr360-intricate-fantasy-art-dark-forest-vr360-c4d-computer-r_8442285_equirectangular-png.png"
												)
											);
											Done.Execute();
										}
									);
								}
							);

							LatentIt(
								"Should return 404 if no id was provided",
								EAsyncExecution::ThreadPool,
								[this](const FDoneDelegate& Done)
								{
									ApiInstance.Get()->Skybox()->GetExportById(
										TEXT(""),
										[this, Done](
										FSkyboxExportResponse* Response,
										int StatusCode,
										bool bConnectedSuccessfully)
										{
											TestNotNull("Response should not be null", Response);
											TestEqual("StatusCode should be 200", StatusCode, 404);
											TestEqual("Status should be failed", Response->status, TEXT("failed"));
											TestEqual("Error should say invalid id", Response->error_message, TEXT("Invalid ID provided"));
											Done.Execute();
										}
									);
								}
							);
						}
					);

					Describe(
						"PostExport",
						[this]()
						{
							LatentIt(
								"Should be able to create an export",
								EAsyncExecution::ThreadPool,
								[this](const FDoneDelegate& Done)
								{
									ApiInstance.Get()->Skybox()->PostExport(
										FSkyboxExportRequest(),
										[this, Done](
										FSkyboxExportResponse* Response,
										int StatusCode,
										bool bConnectedSuccessfully)
										{
											TestNotNull("Response should not be null", Response);
											TestEqual("StatusCode should be 200", StatusCode, 200);
											TestTrue("bConnectedSuccessfully should be true", bConnectedSuccessfully);
											TestEqual("Correct obfuscated id", Response->id, TEXT("b26584eb869cdebeecdf009b7c17c73b"));
											TestEqual("Correct status", Response->status, TEXT("complete"));
											TestEqual(
												"Correct file URL",
												Response->file_url,
												TEXT(
													"https://blockade-platform-production.s3.amazonaws.com/exports/equirectangular-png/vr360-intricate-fantasy-art-dark-forest-vr360-c4d-computer-r_8442285_equirectangular-png.png"
												)
											);
											Done.Execute();
										}
									);
								}
							);
						}
					);
				}
			);

			Describe(
				"ImagineProvider",
				[this]
				{
					Describe(
						"GetRequests",
						[this]()
						{
							LatentIt(
								"Should be able to get a list of requests",
								EAsyncExecution::ThreadPool,
								[this](const FDoneDelegate& Done)
								{
									ApiInstance.Get()->Imagine()->GetRequests(
										46,
										[this, Done](
										FImagineGetExportsResponseRequest* Response,
										int StatusCode,
										bool bConnectedSuccessfully)
										{
											TestNotNull("Response should not be null", Response);
											TestEqual("StatusCode should be 200", StatusCode, 200);
											TestTrue("bConnectedSuccessfully should be true", bConnectedSuccessfully);
											TestEqual("Correct obfuscated id", Response->obfuscated_id, TEXT("c60354672339614111cee85d21253244"));
											TestEqual("Correct status", Response->status, TEXT("complete"));
											Done.Execute();
										}
									);
								}
							);
						}
					);

					Describe(
						"GetRequestObfuscatedId",
						[this]()
						{
							LatentIt(
								"Should be able to get a list of requests",
								EAsyncExecution::ThreadPool,
								[this](const FDoneDelegate& Done)
								{
									ApiInstance->Imagine()->GetRequestsObfuscatedId(
										"c60354672339614111cee85d21253244",
										[this, Done](
										FImagineGetExportsResponseRequest* Response,
										int StatusCode,
										bool bConnectedSuccessfully)
										{
											TestNotNull("Response should not be null", Response);
											TestEqual("StatusCode should be 200", StatusCode, 200);
											TestTrue("bConnectedSuccessfully should be true", bConnectedSuccessfully);
											TestEqual("Correct obfuscated id", Response->obfuscated_id, TEXT("c60354672339614111cee85d21253244"));
											TestEqual("Correct status", Response->status, TEXT("complete"));
											Done.Execute();
										}
									);
								}
							);

							LatentIt(
								"Should return 404 if no id was provided",
								EAsyncExecution::ThreadPool,
								[this](const FDoneDelegate& Done)
								{
									ApiInstance.Get()->Imagine()->GetRequestsObfuscatedId(
										TEXT(""),
										[this, Done](
										FImagineGetExportsResponseRequest* Response,
										int StatusCode,
										bool bConnectedSuccessfully)
										{
											TestNotNull("Response should not be null", Response);
											TestEqual("StatusCode should be 200", StatusCode, 404);
											TestEqual("Status should be failed", Response->status, TEXT("failed"));
											TestEqual("Error should say invalid id", Response->error_message, TEXT("Invalid ID provided"));
											Done.Execute();
										}
									);
								}
							);
						}
					);
				}
			);
		}
	);
}
