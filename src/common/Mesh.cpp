#include "Mesh.hpp"
#include "GraphicAPI.hpp"
#include "DEngine.hpp"

Mesh::Mesh()
{
    mask = 0;
}

Mesh::Mesh(vector<Vertex>vs, vector<unsigned int> ids, vector<string> texns, int mask):
vs(vs), ids(ids), texns(texns), mask(mask)
{
    // // ���ض����Լ���������
    // GraphicAPI::LoadMesh(*this);
    // // ����������ͼ
    // texs.resize(aiTextureType_UNKNOWN + 1);
    // for (int it = aiTextureType_NONE; it <= aiTextureType_UNKNOWN; it++) {
    //     if (mask & (1 << it)) {
    //         texs[it] = DEngine::GetTexMgr().GetTextureByFileName(texns[it]);
    //     }
    // }
}

Grid::Grid(vec2  a, vec2 b, vec2 c, vec2 d, int lines)
{
    vec2 nx = normalize(b-a);
    vec2 ny = normalize(d-a);
    vec2 dx = nx * glm::length(b-a)/(float)(lines-1);
    vec2 dy = ny * glm::length(d-a)/(float)(lines-1);
    vs.resize(lines*4);
    // ��
    vec2 up = a;
    vec2 down = d;
    for(int i = 0; i < lines; i++){
        vs[2*i].vertex = (vec3(up.x,0,up.y));
        vs[2*i+1].vertex = (vec3(down.x,0,down.y));
        up += dx;
        down += dx;
    }
    // ��
    vec2 left = a;
    vec2 right = b;
    for(int i = 0; i < lines; i++){
        vs[2*i+2*lines].vertex = (vec3(left.x,0,left.y));
        vs[2*i+1+2*lines].vertex = (vec3(right.x,0,right.y));
        left += dy;
        right += dy;
    }
    ids.resize(vs.size());
    for(int i = 0; i < vs.size(); i++) ids[i] = i;

    GraphicAPI::LoadMesh(*this);
}

Panel::Panel(vec3 a, vec3 b, vec3 c, vec3 d)
{
    vs.resize(4);
    vs[0].vertex = a;
    vs[1].vertex = b;
    vs[2].vertex = c;
    vs[3].vertex = d;
    for(Vertex& v: vs) v.normal = vec3(0, 1.0, 0);

    ids.resize(6);
    // a, d, b
    ids[0] = 0;
    ids[1] = 3;
    ids[2] = 1;
    // d, c, b
    ids[3] = 3;
    ids[4] = 2;
    ids[5] = 1;
}

SkyBox::SkyBox(float up, float down, float left, float right, float front, float back)
{
    vec3 a[8];
    a[0] = vec3(left, up, front);
    a[1] = vec3(left, up, back);
    a[2] = vec3(right, up, front);
    a[3] = vec3(right, up, back);
    a[4] = vec3(left, down, front);
    a[5] = vec3(left, down, back);
    a[6] = vec3(right, down, front);
    a[7] = vec3(right, down, back);

    vs.resize(8);
    for(int i = 0; i < 8; i++){
        vs[i].vertex = a[i];
    }
    ids = {
        1,0,2,
        2,3,1,
        1,5,7,
        7,3,1,
        0,4,6,
        6,2,0,
        5,4,6,
        6,7,5,
        0,4,5,
        5,1,0,
        2,6,7,
        7,3,2
    };
}

Frustum::Frustum(float fov, float aspect, float n, float f, int xCnt, int yCnt, int zCnt)
{
    float tangent = tan(fov*0.5);
    // tangent = height*0.5 / n
    // height = n*tangent*2
    // width / height = aspect
    // width = height*aspect
    float height = 2.0*n*tangent;
    float width = height*aspect;
    // far_height / height = f/n
    // far_height = height*f/n
    // far_width = far_height*aspect
    float far_height = height * f / n;
    float far_width = far_height*aspect;
    vs.clear();
    ids.clear();
    for(int i = 1; i <= zCnt+1; i++){
        float curZ = (f-n)/(float)zCnt * (float)(i-1) + n;
        float rate = curZ / n;
        float curX = -rate*width/2.0;
        float stepX = rate*width/(float)xCnt;
        float stepY = rate*height/(float)yCnt;
        for(int x = 1; x <= xCnt+1; x++){
            float curY = -rate*height/2.0;
            for(int y = 1; y <= yCnt+1; y++){
                int id = vs.size();
                Vertex ver = Vertex(vec3(curX, curY, -curZ));
                ver.x = xCnt+1 - x;
                ver.y = yCnt+1 - y;
                ver.z = i - 1;
                vs.push_back(ver);
                int leftId = id - 1;
                int upId = id - yCnt - 1;
                int preId = id - (xCnt+1)*(yCnt+1);
                if(x!=1){
                    ids.push_back(id);
                    ids.push_back(upId);
                }
                if(y!=1){
                    ids.push_back(id);
                    ids.push_back(leftId);
                }
                if(i!=1){
                    ids.push_back(id);
                    ids.push_back(preId);
                }
                curY += stepY;
            }
            curX += stepX;
        }
    }
}


