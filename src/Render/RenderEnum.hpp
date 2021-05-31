#pragma once

enum ResourceState {
    read,
    write,
    create,
};

enum ViewType {
    CBView,
    SRView,
    UAView,
    RTView,
};

enum ShaderType {
    CS,
    VS,
    GS,
    PS,
};