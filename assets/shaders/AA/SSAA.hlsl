Texture2D gPixMap: register(t0);

struct VertexIn
{
	float3 vertex: POSITION;
};

struct VertexOut
{
	float4 pos: SV_POSITION;
};

// 0/5  1/ 
// 4  2/3
#define vpWidth 1334.0
#define vpHeight 750.0
#define ssRate 4

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
    int x = pos.x*ssRate;
    int y = pos.y*ssRate;
    float4 sum = float4(0.0, 0.0, 0.0, 0.0);
    for(int i = 0; i < ssRate; i++){
        for(int j = 0; j < ssRate; j++){
            sum = sum + gPixMap.Load(int3(x+i,y+j,0));
        }
    }

    sum = sum / ssRate / ssRate;
        
    return sum;
}