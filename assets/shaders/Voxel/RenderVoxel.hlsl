cbuffer VoxelConstant : register(b0)
{
    int _width;
    int _height;
    int _depth;
    int _sizeX;
    int _sizeY;
    int _sizeZ;
    float4x4 _orthoProj;
};

cbuffer PassConstant: register(b1)
{
	float4x4 _View;
	float4x4 _Proj;
	float4x4 _SMView;
	float4x4 _SMProj;
	float4x4 _JProj;
	float4x4 _lastView;
	float4x4 _lastProj;
	float3 _CamPos;
};

struct VertexIn
{
	float3 vertex: POSITION;
	float3 normal: NORMAL;
    float2 texcoord: TEXCOORD;
	float3 tangent: TANGENT;
	float3 bitangent: TANGENT1;
};

struct VertexOut
{
    float4 pos: SV_POSITION;
};

// struct GeoOut {
//     float4 pos: SV_POSITION;
//     float4 color: TEXCOORD;
// };

RWTexture3D<float4> voxelGrid: register(u0);

VertexOut VS(VertexIn vin, uint id: SV_VertexID)
{
	VertexOut vout;
	float4x4 model;
	for(int i = 0; i <  4; i++){
		for(int j = 0; j < 4; j++){
			if(i==j){
				if(i==3) model[i][j] = 1.0;
				else model[i][j] = 100.0;
			}
			else
				model[i][j] = 0.0;
		}
	}
	float4x4 mvp = mul(mul(_Proj, _View), model);
	vout.pos = mul(mvp, float4(vin.vertex, 1.0f));

    return vout;
	// VertexOut vout;
	// if(id == 0 || id==5){
    //     vout.pos = float4(-1.0, 1.0, 0.0, 1.0);
    // }
    // else if(id==1){
    //     vout.pos = float4(1.0, 1.0, 0.0, 1.0);
    // }
    // else if(id==2 || id==3){
    //     vout.pos = float4(1.0, -1.0, 0.0, 1.0);
    // }
    // else{
    //     vout.pos = float4(-1.0, -1.0, 0.0, 1.0);
    // }
    return vout;
}

// void EmmitGrid(int3 offset, int step, inout TriangleStream<GeoOut> stream)
// {


//     for(int i = 0; i < 3; i++){
//         float ver[3];
//         ver[0] = vertices[i*3];// * step + offset;
//         ver[1] = vertices[i*3+1];// * step + offset;
//         ver[2] = vertices[i*3+2];// * step + offset;

//         GeoOut gout;
//         float4x4 vp = mul(_Proj, _View);
//         gout.pos = float4(ver[0], ver[1], ver[2], -2.0);
//         gout.color = float4(1.0, 1.0, 1.0, 1.0);
//         stream.Append(gout);
//     }
// }

// [maxvertexcount(3)]
// void GS(triangle VertexOut gin[3], inout TriangleStream<GeoOut> stream)
// {
//     for(int i = 0; i < 1; i++){
//         uint row = gin[i].voxelIndex % _width;
//         uint col = (gin[i].voxelIndex/_width) %  _width;
//         uint dep = (gin[i].voxelIndex/_width/_width) % _width;

//         int stepX = _sizeX / _width;
//         int stepY = _sizeY / _height;
//         int stepZ = _sizeZ / _depth;
//         int offsetX = stepX * row;
//         int offsetY = stepY * col;
//         int offsetZ = stepZ * dep;

//         EmmitGrid(int3(offsetX, offsetY, offsetZ), stepX, stream);
//     }
// }

float4 PS(VertexOut pin): SV_TARGET
{
    return float4(1.0, 1.0, 1.0, 1.0);
}