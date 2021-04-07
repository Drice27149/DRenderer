#include "Renderer.hpp"

Renderer::Renderer()
{

}

void Renderer::InitRender()
{
    // shadow map
    glGenFramebuffers(1, &shadowFBO);
    glGenTextures(1, &shadowTex);
    glBindTexture(GL_TEXTURE_2D, shadowTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    // 纹理过滤
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); 
    // bind texture to fbo
    glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
    glBindTexture(GL_TEXTURE_2D, shadowTex);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowTex, 0);
    glDrawBuffer(GL_NONE); 
    glReadBuffer(GL_NONE);        
    // ssao map
    glGenFramebuffers(1, &ssaoFBO);
    glGenTextures(1, &ssaoTex);
    glBindTexture(GL_TEXTURE_2D, ssaoTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
    //glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_FLOAT, NULL);
    // stuff
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); 
    // bind texture to fbo
    glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoTex, 0);
    // todo: attach depth buffer


    // GBuffer
    glGenFramebuffers(1, &gFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, gFBO);
    glGenTextures(1, &coordTex);
    glBindTexture(GL_TEXTURE_2D, coordTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
    // stuff
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); 
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, coordTex, 0);
    // normal buffer
    glGenTextures(1, &normalTex);
    glBindTexture(GL_TEXTURE_2D, normalTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
    // stuff
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); 
    // bind texture to fbo
    glBindFramebuffer(GL_FRAMEBUFFER, gFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, normalTex, 0);
    // diffuse and specular buffer
    glGenTextures(1, &colorTex);
    glBindTexture(GL_TEXTURE_2D, colorTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
    // stuff
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); 
    // bind texture to fbo
    glBindFramebuffer(GL_FRAMEBUFFER, gFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, colorTex, 0);
    // bind attachments
    vector<unsigned int> attachments = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
    glDrawBuffers(3, attachments.data());
    // depth/render buffer
    unsigned int rboDepth;
    glGenRenderbuffers(1, &rboDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);
    // unbind
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


void Renderer::SSAOPass(Shader* shader)
{
    shader->use();
    glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    // GBuffer
    shader->setVec2("screenSize", vec2(width, height));
    shader->setInt("gPosition", 3);
    shader->setInt("gNormal", 4);
    shader->setInt("gColor", 5);
    // init ssao sampler
    for(int i = 0; i < 64; i++){
        std::string name = "RandomSample[" + std::to_string(i) + "]";
        shader->setVec3(name.c_str(), ssaoKernel[i]);
    }
    // render
    glm::mat4 trans = projection * camera->getCamTransform();
    shader->setMat4("trans", trans);
    for(Object* object: scene->objects){
        object->draw(shader);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glActiveTexture(GL_TEXTURE6);
    glBindTexture(GL_TEXTURE_2D, ssaoTex);
}

void Renderer::GPass(Shader* shader)
{
    shader->use();
    glBindFramebuffer(GL_FRAMEBUFFER, gFBO);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    glm::mat4 view = camera->getCamTransform();
    glm::mat4 trans = projection * view;
    shader->setMat4("trans", trans);
    shader->setVec3("eyePos",camera->getOrigin());
    shader->setVec3("inten", scene->lights[0][2]);
    shader->setVec3("inten_a", scene->inten_a);
    shader->setVec3("lightPos", scene->lights[0][0]);
    for(Object* object: scene->objects){
        object->draw(shader);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    // save result
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, coordTex);
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, normalTex);
    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, colorTex);
}

void Renderer::ShadowPass(Shader* shader)
{
    shader->use();
    glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
    // glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
    glClear(GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST); 
    Camera lightCam(scene->lights[0][0], scene->lights[0][1]);
    glm::mat4 trans =  projection * lightCam.getCamTransform();
    shader->setMat4("trans", trans);
    for(Object* object: scene->objects){
        object->draw(shader);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    // save shadow map texture 
    glActiveTexture(GL_TEXTURE1); 
    glBindTexture(GL_TEXTURE_2D, shadowTex);
    // // stuff
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor); 
}

void Renderer::Render(Shader* shader)
{
    shader->use();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    // assume only one light here
    // {position, direction, intensity}
    // transform related to camera and eye 
    // init texture
    shader->setInt("depthMap", 1);
    shader->setInt("ssaoMap", 6);
    shader->setInt("gPostion", 3);
    shader->setInt("gNormal", 4);
    shader->setInt("gColor", 5);
    // init vector
    glm::mat4 view = camera->getCamTransform();
    glm::mat4 trans = projection * view;
    shader->setMat4("trans", trans);
    shader->setVec3("eyePos",camera->getOrigin());
    shader->setVec3("inten", scene->lights[0][2]);
    shader->setVec3("inten_a", scene->inten_a);
    shader->setVec3("lightPos", scene->lights[0][0]);
    shader->setVec2("screenSize", vec2(scene->width, scene->height));
    Camera lightCam(scene->lights[0][0], scene->lights[0][1]);
    glm::mat4 lightTrans =  projection * lightCam.getCamTransform();
    shader->setMat4("lightTrans", lightTrans);
    for(Object* object: scene->objects){
        object->draw(shader);
    }
}

float lerp(float a, float b, float f)
{
    return a + f * (b - a);
}

void Renderer::InitSSAO()
{
    std::uniform_real_distribution<GLfloat> randomFloats(0.0, 1.0); // generates random floats between 0.0 and 1.0
    std::default_random_engine generator;
    for (unsigned int i = 0; i < 64; ++i)
    {
        glm::vec3 sample(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, randomFloats(generator));
        sample = glm::normalize(sample);
        sample *= randomFloats(generator);
        float scale = float(i) / 64.0;

        // scale samples s.t. they're more aligned to center of kernel
        scale = lerp(0.1f, 1.0f, scale * scale);
        sample *= scale;
        ssaoKernel.push_back(sample);
    }
}