#include "MockSkyboxAiHttpClient.h"

UMockSkyboxAiHttpClient::UMockSkyboxAiHttpClient()
{
	MockOutPath = FPaths::Combine(FPaths::ProjectDir(), "Plugins/BlockadeLabsSkyboxAi/Source/SkyboxAiApi/Private/Specs/Mocks/Responses/");
}

void UMockSkyboxAiHttpClient::MakeAPIRequest(const FString& Endpoint, const FSkyboxAiHttpHeaders& Headers, const FString& Body, FSkyboxAiHttpCallback Callback)
{
	if (Endpoint == "/skybox" && Headers.Method == SkyboxAiHttpClientDefinitions::HTTPVerbs::GPost)
	{
		Callback(LoadMockResponse("skybox-post.json"), 200, true);
	}
	else if (Endpoint == "/skybox/styles" && Headers.Method == SkyboxAiHttpClientDefinitions::HTTPVerbs::GGet)
	{
		Callback(LoadMockResponse("skybox-styles.json"), 200, true);
	}
	else if (Endpoint == "/skybox/export" && Headers.Method == SkyboxAiHttpClientDefinitions::HTTPVerbs::GGet)
	{
		Callback(LoadMockResponse("skybox-export.json"), 200, true);
	}
	else if (Endpoint == "/skybox/export" && Headers.Method == SkyboxAiHttpClientDefinitions::HTTPVerbs::GPost)
	{
		Callback(LoadMockResponse("skybox-export-post.json"), 200, true);
	}
	else if (Endpoint == "/skybox/export/b26584eb869cdebeecdf009b7c17c73b" && Headers.Method == SkyboxAiHttpClientDefinitions::HTTPVerbs::GGet)
	{
		Callback(LoadMockResponse("skybox-export-id.json"), 200, true);
	}
	else if (Endpoint == "/imagine/requests/46" && Headers.Method == SkyboxAiHttpClientDefinitions::HTTPVerbs::GGet)
	{
		Callback(LoadMockResponse("imagine-requests-id.json"), 200, true);
	}
	else if (Endpoint == "/imagine/requests/obfuscated-id/c60354672339614111cee85d21253244" && Headers.Method == SkyboxAiHttpClientDefinitions::HTTPVerbs::GGet)
	{
		Callback(LoadMockResponse("imagine-requests-obfuscated-id.json"), 200, true);
	}
}

FString UMockSkyboxAiHttpClient::LoadMockResponse(const FString& Filename) const
{
	FString Response;
	FFileHelper::LoadFileToString(Response, *FPaths::Combine(MockOutPath, Filename));
	return Response.Replace(TEXT("\r"), TEXT("")).Replace(TEXT("\n"), TEXT(""));
}
