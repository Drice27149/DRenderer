#pragma once

namespace ResourceEnum{

enum State {
    Read,
    Write,
    Create,
};

enum Type {
    Buffer,
    Constant,
    Texture2D,
    Texture3D,
    TextureCube,
    DepthStencil,
};

enum Format {
    R8G8B8A8_UNORM,
    R32G32B32A32_FLOAT,
    R16G16_FLOAT,
    R32G32B32A32_UINT,
    R32_UINT,
};

enum View {
    CBView,
    SRView,
    DSView,
    UAView,
    RTView,
    UKnownView,
};

enum Usage {
    
};

}

namespace ShaderEnum {

enum Type {
    CS,
    VS,
    GS,
    PS,
};

}

namespace PSOEnum {

enum BlendState {
    Default,
};

};