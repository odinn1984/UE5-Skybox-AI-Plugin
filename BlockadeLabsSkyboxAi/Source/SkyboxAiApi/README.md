# SkyboxAI API Implementation

This is a simple implementation of an HTTP client that we can pass as a dependency to the API implementation

All of the calls here rely on callbacks (C++ Lambdas) since there are no more than a couple of levels of chaining them one to another it keeps the implementatin simpler rather than relying on other syncing mechanism like delegates.

If you are not familiar with C++ Lambdas please read [this document](https://en.cppreference.com/w/cpp/language/lambda)

For more information about the SkyboxAI API please go to their [online docs](https://api-documentation.blockadelabs.com/api/)

## Table Of Content

- [SkyboxAI API Implementation](#skyboxai-api-implementation)
  - [Table Of Content](#table-of-content)
  - [API Implementation](#api-implementation)
  - [Providers](#providers)
    - [Skybox Provider](#skybox-provider)
    - [Imagine Provider](#imagine-provider)

## API Implementation

Simple and very specific implementation of an API client for SkyboxAI that implements only the required endpoints for this plugin

It has 2 providers that you can access which are `Skybox` and `Imagine`

You can also set the HTTP client that you want to use, by default it uses the `USKyboxAiHttpClient` which is a simple implementation of an HTTP client that uses the UE HTTP module

You can set the client to any class that inherits from `USkyboxAiHttpClientBase`

This is a very modular API client that you can add or modify providers as needed without needing to add functionality

## Providers

The API implementation takes the composition over inheritance approach and a lite version of dependency injection to create the functionality to interact with SkyboxAI API

This is achieved by implementing providers, providers are just resources from the API, e.g: skybox and imagine

### Skybox Provider

This provider implements logic for the following endpoints:

| Method | Endpoint                                                                                                     | Description                                     |
| ------ | ------------------------------------------------------------------------------------------------------------ | ----------------------------------------------- |
| POST   | [/skybox](https://api-documentation.blockadelabs.com/api/skybox.html#generate-skybox)                        | Send a request to generate a skybox             |
| GET    | [/skybox/styles](https://api-documentation.blockadelabs.com/api/skybox.html#get-skybox-styles)               | Get all supported styles for skybox generation  |
| GET    | [/skybox/export](https://api-documentation.blockadelabs.com/api/skybox-exports.html#get-export-types)        | Get all possible export formats                 |
| GET    | [/skybox/export/{id}](https://api-documentation.blockadelabs.com/api/skybox-exports.html#get-export-request) | Get the status of the export with a specific ID |
| POST   | [/skybox/export](https://api-documentation.blockadelabs.com/api/skybox-exports.html#request-export)          | Request an export of the skybox into a file     |

### Imagine Provider

This provider implements logic for the following endpoints:

| Method | Endpoint                                                                                                                       | Description                                                                                                               |
| ------ | ------------------------------------------------------------------------------------------------------------------------------ | ------------------------------------------------------------------------------------------------------------------------- |
| GET    | [/imagine/requests/{id}](https://api-documentation.blockadelabs.com/api/skybox.html#get-skybox-by-id) | Used to implement the functionality of import |
| GET    | [/imagine/requests/obfuscated-id/{id}](https://api-documentation.blockadelabs.com/api/skybox.html#get-skybox-by-obfuscated-id) | Used to poll the generation process of our skybox so that once it's generated we can export it and save it to our project |
