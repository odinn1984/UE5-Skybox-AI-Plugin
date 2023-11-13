# Blockade Labs SkyboxAI Plugin For UE5

This is a tool that integrates with [SkyboxAI](https://skybox.blockadelabs.com/) by [Blockade Labs](https://www.blockadelabs.com/)

**This is not an official plugin made by Blockade Labs, it's a community plugin**. To see official SDK's and Libraries please visit [Blockade Labs API Documentation](https://api-documentation.blockadelabs.com/api/sdk.html)

The purpose of this plugin is to allow you to request to generate a sky box and then automatically download it and put it in your project with an option to disable features that are premium (to reduce UI clutter and show only things that you can use)

If you are looking for information about the implementation of the API that is used in this plugin please visit the [API docs](./BlockadeLabsSkyboxAi/Source/SkyboxAiApi/README.md)

If you are interested in knowing more about Blockade Labs and Skybox AI please to go the [Links And Resources](#links-and-resources) section

## Table Of Content

- [Blockade Labs SkyboxAI Plugin For UE5](#blockade-labs-skyboxai-plugin-for-ue5)
  - [Table Of Content](#table-of-content)
  - [Requirements](#requirements)
  - [Important Notes](#important-notes)
  - [Supported Platforms](#supported-platforms)
  - [Setting Up The Plugin](#setting-up-the-plugin)
    - [Plugin Settings](#plugin-settings)
  - [Using The Plugin](#using-the-plugin)
    - [Generating a Skybox](#generating-a-skybox)
    - [Importing Skybox Data](#importing-skybox-data)
    - [Remixing a Skybox](#remixing-a-skybox)
  - [Setting Up Dev Environment](#setting-up-dev-environment)
    - [Coding Standards](#coding-standards)
  - [Limitations](#limitations)
  - [Submitting Issues / Feature Requests](#submitting-issues--feature-requests)
  - [Links And Resources](#links-and-resources)

## Requirements

For this plugin to work you will need to have a SkyboxAI account and an API key, please visit [SkyboxAI API Membership](https://skybox.blockadelabs.com/api-membership) page to see their pricing and create an account

You might need to install a specific .NET runtime depending on your UE version

## Important Notes

When using this plugin it introduces new editor level settings where you put your API key, UE will create a file for this config under `[PROJECT ROOT]/Config/DefaultBlockadeLabs.ini`

Since this file contains **SENSITIVE INFORMATION** please make sure to **NEVER** submit it anywhere (like GitHub, etc...)

There are many solutions for secret management, please use one of them if you want to use this plugin as part of some automatic process

## Supported Platforms

| **Engine Version** | **Windows** | **Linux** | **OSX** |
| ------------------ | ----------- | --------- | ------- |
| **5.0**            | ✔️           | ❌         | ❌       |
| **5.1**            | ✔️           | ❌         | ❌       |
| **5.2**            | ✔️           | ❌         | ❌       |
| **5.3**            | ✔️           | ❌         | ❌       |

## Setting Up The Plugin

To setup the plugin please follow the following steps:

1. Either clone this repository or download the zipped source code
2. Copy the `BlockadeLabsSkyboxAi` directory into one of the following paths
   1. To the `Plugins` folder of your UE5 project
   2. To the `Plugins` folder of your engine version (e.g: `D:\Unreal Engines\UE_5.3\Engine\Plugins`)
3. Open your project and go to `Edit` -> `Plugins`
4. Search for`BlockadeLabs SkyboxAI` which will be under the `AI` category either under `BUILT-IN` or `PROJECT` depending on where you put the folder of the plugin
5. Enable the plugin and restart the engine
6. After the restart the plugin will be under the `Tools` menu and will be called `Blockade Labs SkyboxAI`
7. Please go to `Edit` -> `Editor Settings` and put in the API Key that you have generated.
   1. To generate and API key you will need a SkyboxAI account and then go to [API Settings](https://skybox.blockadelabs.com/profile/api) and press on the `Generate API Key` button or use a currently available API key

### Plugin Settings

- `API Key` - This is where you put your API key, this is a sensitive information so please make sure to not commit it anywhere
- `API Endpoint` - You can customize the endpoint of the API in case you are using a different one [Default: https://backend.blockadelabs.com/api/v1]
- `API Polling Interval In Seconds` - This allows you to change the frequency of the polling of the API in case of rate limiting issues [Default: 1.0]
- `Enable Premium Content` - This tells the plugin if we want to send and get information that is marked as premium (e.g: Enhanced Prompt) [Default: true]
- `Save Directory` - This is where the plugin will save the skybox that it downloads from the API, if the directory doesn't exist it will be created [Defauklt: SkyboxAI/Exports]
  - This is a relative path to the project's `Content` directory
  - Notice that it doesn't start with `/` or `\` and it doesn't end with `/` or `\`

## Using The Plugin

For more information what `Prompt`, `Negative Text` and `Enhanced Prompt` mean please go to the [SkyboxAI API docs](https://api-documentation.blockadelabs.com/api/skybox.html#generate-skybox)

### Generating a Skybox

In order to generate a skybox you need to follow these steps:

1. Go to `Tools` -> `Blockade Labs SkyboxAI`
2. Enter the description of the skybox that you want to generate in `Prompt` field
3. Optionally fill in the `Negative Text` field if you want to add some negative text for the generator to use
4. You must select the export format (e.g: `HDR` or `EXR`) in the top list on the right
5. Optionally you can select the style of the skybox in the bottom list on the right
   1. By default the first style is selected from the list on first load
   2. You can click on any selected style to de-select it if you do not want to send in style information
6. Optionally you can also enable the `Enhanced Prompt` checkbox if you want to use the enhance prompt feature
7. The `Refresh List` button will refresh the list of styles and export types
8. Press on `Generate Skybox` and wait, please do not close the plugin tab while it's generating the skybox. The plugin will notify you on progress and when it's done
9.  At the end, it will download the image and put it in the directory that you have specified in the plugin settings and UE will give you a prompt asking if you want to re-import the asset since the directory it monitors has changed. Please re-import the assets and follow the instructions for that process that UE provides

### Importing Skybox Data

In order to import a skybox you need to follow these steps:

1. Go to your [Dashboard](https://skybox.blockadelabs.com/profile/history) on SkyboxAI
2. Choose the skybox that you want to remix and and click on it
   1. You can get the details that this image was generated with (if for example you want to use the same prompt or negative text etc...)
   2. Clink on "View" and it will open this image in a new tab and copy the title of the image (e.g: if the title is `World #1234` then `1234` is the ID of the image)
3. Go to `Tools` -> `Blockade Labs SkyboxAI`
4. Click on the `Import...` button
5. Fill in the ID that you have copied from the URL in the prompt that will show up and press "Confirm"
6. This will fill in the Style, Prompt and Negative Text fields with the information that was used to generate the skybox with the ID that you have provided

### Remixing a Skybox

In order to remix a skybox you need to follow these steps:

1. Go to your [Dashboard](https://skybox.blockadelabs.com/profile/history) on SkyboxAI
2. Choose the skybox that you want to remix and and click on it
   1. You can get the details that this image was generated with (if for example you want to use the same prompt or negative text etc...)
   2. Clink on "View" and it will open this image in a new tab and copy the title of the image (e.g: if the title is `World #1234` then `1234` is the ID of the image)
3. Go to `Tools` -> `Blockade Labs SkyboxAI`
4. Fill in all the information that you want to send just as if you were creating a new skybox
   1. This part can be skipped if you have imported a skybox and the information was filled in automatically
5. Click on the `Remix...` button
6. Fill in the ID that you have copied from the URL in the prompt that will show up and press "Confirm"
7. The rest will be the same as generating a new skybox

## Setting Up Dev Environment

To setup a development environment please follow these steps:

1. Create a new project using one of the supported engine versions
   1. If you want to introduce support for an engine versions that is not listed, contribution to this repository is very appreciated!
2. Continue with following the steps [above](#setting-up-the-plugin) and in step number 2 put the plugin in your project's `Plugins` directory

### Coding Standards

Please make sure to follow the following rules when writing code for this repository

1. Please use the format defined in `.clang-format` (These are a bit different spacing wise and line-break wise than what UE has in their code, personal preference)
2. Please make sure to follow the naming conventions in [unreal docs](https://docs.unrealengine.com/5.3/en-US/epic-cplusplus-coding-standard-for-unreal-engine/)
3. If you are adding functionality or modifying something please consider adding tests as well where possible, this will help us make sure that we don't break anything in the future
4. Try to not go overboard with testing every single line of code, code coverage is less important than functionality and user flow coverage
5. Please make sure that the change is running on all supported engine versions
   1. Hopefully I will add automation around that so that we can have a testing matrix, but until then we have to make sure we don't break any engine version
6. Oh yeah... and please refrain from putting comments in code, comments are a mess... If something is unclear let's refactor it so that reading the code makes sense and we can understand what it does without needing to put comments
   1. Like everything, this also has exception so please don't follow this rule religiously, if you feel like you need to put a comment, put it, but please try to avoid it as much as possible

## Limitations

This plugin is currently missing a few features that are available on SkyboxAI, please see the full list of features and what is available and what is missing

- [x] Generate a Skybox and export to a file
- [x] Remix a Skybox
- [x] Import a Skybox
- [ ] Download depth map [On the TODO list]
- [ ] 3D features
- [ ] Control image
- [ ] Preview

Support for Pusher / Webhooks will not be implemented

If any of the missing features will be required I am happy to implement them or get contributions for them

Any information about generated Skyboxes and history of usage can be found in the [SkyboxAI's Dashboard](https://skybox.blockadelabs.com/profile/history)

## Submitting Issues / Feature Requests

If you have any issues with this plugin please submit them in the [Issues](https://github.com/odinn1984/UE5-Skybox-AI-Plugin/issues) section of this repository

If you have any feature requests please submit them in the [Feature Requests](https://github.com/odinn1984/UE5-Skybox-AI-Plugin/discussions/categories/suggestions-feature-requests) section of this repository

Please review the [Q&A](https://github.com/odinn1984/UE5-Skybox-AI-Plugin/discussions/categories/q-a) section of this repository before submitting any issues or feature requests

For general discussions please visit the [General](https://github.com/odinn1984/UE5-Skybox-AI-Plugin/discussions/categories/general) section of this repository

## Links And Resources

[Blockade Labs](https://www.blockadelabs.com/)<br />
[SkyboxAI](https://skybox.blockadelabs.com/)<br />
[SkyboxAI API Docs](https://api-documentation.blockadelabs.com/api/)<br />
