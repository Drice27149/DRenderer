#include "AssimpLoader.hpp"

AssimpLoader::AssimpLoader()
{
    meshCnt = 0;
    have_vn = have_vt = 0;
}

void AssimpLoader::LoadFile(string fn)
{
    Assimp::Importer import;
    const aiScene *scene = import.ReadFile(fn, aiProcess_Triangulate | aiProcess_FlipUVs);	
	
    if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) 
    {
        cout << "ERROR::ASSIMP::" << import.GetErrorString() << "\n";
        return;
    }

    ProcessNode(scene->mRootNode, scene);
}

void AssimpLoader::ProcessNode(aiNode *node, const aiScene *scene)
{
    for(int i = 0; i < node->mNumMeshes; i++){
        aiMesh* cm = scene->mMeshes[node->mMeshes[i]];
        ProcessMesh(cm, scene);
    }
    for(int i = 0; i < node->mNumChildren; i++){
        ProcessNode(node->mChildren[i], scene);
    }
}

void AssimpLoader::ProcessMesh(aiMesh *mesh, const aiScene *scene)
{
    // 只有 mVertice 和 mFace 是总存在的, 其他属性都可能为 null
    // 不同的 mesh 顶点索引和纹理贴图是不一样的
    meshCnt++;
    for(int i = 0; i < mesh->mNumVertices; i++){
        Vertex v;
        v.v = vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
        if(mesh->mNormals){ 
            v.vn = vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
        }
        if(mesh->mTextureCoords[0]){
            v.vt = vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
            // TODO: 加载切线
        }
        vs.push_back(v);
    }
    for(int i = 0; i < mesh->mNumFaces; i++){
        for(int j = 0; j < mesh->mFaces[i].mNumIndices; j++){
            ids.push_back(mesh->mFaces[i].mIndices[j]);
        }
    }
    // printf("meshCnt = %d, have_vn = %d, have_vt = %d\n", meshCnt, mesh->mNormals!=nullptr, mesh->mTextureCoords[0]!=nullptr);
    // TODO: 加载纹理
}


