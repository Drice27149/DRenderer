#include "AssimpLoader.hpp"

AssimpLoader::AssimpLoader()
{
    meshCnt = 0;
    have_vn = have_vt = 0;
}

void AssimpLoader::LoadFile(string fn)
{
    // 获得模型文件的文件夹路径
    fpath = "";
    int p = 0;
    while (p < fn.size()) {
        if (fn[p] != '/') fpath.push_back(fn[p++]);
        else {
            fpath.push_back(fn[p++]);
            int np = p;
            while (np < fn.size() && fn[np] != '/') np++;
            if (np == fn.size()) break;
        }
    }

    Assimp::Importer import;
    const aiScene *scene = import.ReadFile(fn, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_ValidateDataStructure);	
	
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
    aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
    ProcessMaterial(material);
}

void AssimpLoader::ProcessMaterial(aiMaterial* mat)
{
    texs.resize(aiTextureType_UNKNOWN + 1);
    // 遍历所有纹理类型, 看是否有对应的纹理贴图
    mask = 0;
    for(int it = aiTextureType_NONE ; it <= aiTextureType_UNKNOWN; it++){
        aiTextureType texType = static_cast<aiTextureType>(it); 
        if(mat->GetTextureCount(texType)){
            mask |= 1 << it;
            // 只取第一张贴图, 因为多出来的大概率是重复的同一个
            aiString texn;
            mat->GetTexture(texType, 0, &texn);
            string fulltexn = fpath + string(texn.C_Str());

            cout << "it = " << it << ", " << fulltexn << "\n";

            texs[it] = fulltexn;
        }
    }
}


