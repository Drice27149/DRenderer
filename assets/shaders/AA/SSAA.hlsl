Texture2D gPixMap: register(t0);

cbuffer AAPassInfo : register(b0)
{
	uint _ssRate;
};

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
    int x = pos.x*_ssRate;
    int y = pos.y*_ssRate;
    float4 sum = float4(0.0, 0.0, 0.0, 0.0);
    for(int i = 0; i < _ssRate; i++){
        for(int j = 0; j < _ssRate; j++){
            sum = sum + gPixMap.Load(int3(x+i,y+j,0));
        }
    }

    sum = sum / _ssRate / _ssRate;
        
    return sum;
}