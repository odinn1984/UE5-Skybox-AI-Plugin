# Blockade Labs Skybox AI Plugin

This is a tool that integrates with [SkyboxAI](https://skybox.blockadelabs.com/) by [Blockade Labs](https://www.blockadelabs.com/)

The purpose of this plugin is to allow you to request to generate a sky box and then automatically download it and put it in your project with an option to disable features that are premium (to reduce UI clutter and show only things that you can use)

If you are looking for information about the implementation of the API that is used in this plugin please visit the [API docs](./BlockadeLabsSkyboxAi/Source/SkyboxAiApi/README.md)

If you are interested in knowing more about Blockade Labs and Skybox AI please to go the [Links And Resources](#links-and-resources) section

## Table Of Content

- [Blockade Labs Skybox AI Plugin](#blockade-labs-skybox-ai-plugin)
  - [Table Of Content](#table-of-content)
  - [Requirements](#requirements)
  - [Important Notes](#important-notes)
  - [Setting Up The Plugin](#setting-up-the-plugin)
    - [Plugin Settings](#plugin-settings)
  - [Setting Up Dev Environment](#setting-up-dev-environment)
  - [Coding Standards](#coding-standards)
  - [Links And Resources](#links-and-resources)

## Requirements

This plugin was tested on the following versions:

- 5.3

## Important Notes

When using this plugin it introduces new editor level settings where you put your API key, UE will create a file for this config under `[PROJECT ROOT]/Config/DefaultBlackdaleLabs.ini`

Since this file contains **SENSITIVE INFORMATION** please make sure to **NEVER** submit it anywhere (like GitHub, etc...)

There are many solutions for secret management, please use one of them if you want to use this plugin as part of some automatic process

## Setting Up The Plugin

To setup the plugin please follow the following steps:

1. Either clone this repository or download the zipped source code
2. Copy the `BlockadeLabsSkyboxAi` directory into one of the following paths
    - To the `Plugins` folder of your UE5 project
    - To the `Plugins` folder of your engine version (e.g: `D:\Unreal Engines\UE_5.3\Engine\Plugins`)
3. Open your project and go to `Edit` -> `Plugins`
4. Search for`BlockadeLabs SkyboxAI` which will be under the `AI` category either under `BUILT-IN` or `PROJECT` depending on where you put the folder of the plugin
5. Enable the plugin and restart the engine
6. After the restart the plugin will be under the `Tools` menu and will be called `Blockade Labs SkyboxAI`
7. Please go to `Edit` -> `Editor Settings` and put in the API Key that you have generated.
    - To generate and API key you will need a SkyboxAI account and then go to [API Settings](https://skybox.blockadelabs.com/profile/api) and press on the `Generate API Key` button or use a currently available API key

### Plugin Settings

- `API Key` - This is where you put your API key, this is a sensitive information so please make sure to not commit it anywhere
- `API Endpoint` - You can customize the endpoint of the API in case you are using a different one [Default: https://backend.blockadelabs.com/api/v1]
- `API Polling Interval In Seconds` - This allows you to change the frequency of the polling of the API in case of rate limiting issues [Default: 1.0]
- `Enable Premium Content` - This tells the plugin if we want to send and get information that is marked as premium (e.g: Enhanced Prompt) [Default: true]
- `Save Directory` - This is where the plugin will save the skybox that it downloads from the API, if the directory doesn't exist it will be created [Defauklt: /Game/Content/SkyboxAI/Exports]

## Setting Up Dev Environment

To setup a development environment please follow these steps:

1. Create a new project using one of the supported engine versions
    - If you want to introduce support for an engine versions that is not listed, contribution to this repository is very appreciated!
2. Continue with following the steps [above](#setting-up-the-plugin) and in step number 2 put the plugin in your project's `Plugins` directory

## Coding Standards

Please make sure to follow the following rules when writing code for this repository

1. Please use the format defined in `.clang-format` (These are a bit different spacing wise and line-break wise than what UE has in their code, personal preference)
2. Please make sure to follow the naming conventions in [unreal docs](https://docs.unrealengine.com/5.3/en-US/epic-cplusplus-coding-standard-for-unreal-engine/)
3. If you are adding functionality or modifying something please make sure to either add new tests or make sure that the current ones cover the changed functionality
4. Try to not go overboard with testing every single line of code, code coverage is less important than functionality and user flow coverage
5. Please make sure that the change is running on all supported engine versions
    - Hopefully I will add automation around that so that we can have a testing matrix, but until then we have to make sure we don't break any engine version

## Links And Resources

[Blockade Labs](https://www.blockadelabs.com/)<br />
[SkyboxAI](https://skybox.blockadelabs.com/)<br />
[SkyboxAI API Docs](https://api-documentation.blockadelabs.com/api/)<br />
