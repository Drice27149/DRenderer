Texture2D gHistoryMap: register(t0);
Texture2D gPixMap: register(t1);

struct VertexIn
{
	float3 vertex: POSITION;
};

struct VertexOut
{
	float4 pos: SV_POSITION;
};

VertexOut VS(VertexIn vin, uint id: SV_VertexID)
{
    VertexOut vout;
	if(id == 0 || id==5){
        vout.pos = float4(-1.0, 1.0, 0.0, 1.0);
    }
    else if(id==1){
        vout.pos = float4(1.0, 1.0, 0.0, 1.0);
    }
    else if(id==2 || id==3){
        vout.pos = float4(1.0, -1.0, 0.0, 1.0);
    }
    else{
        vout.pos = float4(-1.0, -1.0, 0.0, 1.0);
    }
    return vout;
}

float4 PS(VertexOut pin, float4 pos: SV_POSITION): SV_TARGET
{   
    int x = pos.x;
    int y = pos.y;
    float4 sample0 = gPixMap.Load(int3(x,y,0));
    float4 sample1 = gHistoryMap.Load(int3(x,y,0));
        
    return 1.0*sample0+0.0*sample1;
}