### 启动流程

* ```LaunchWindows.cpp: WinMain```

  * ```LaunchWindows.cpp: GuardedMain```, 用于处理抛出的异常 / 错误

    * ```Launch.cpp EnginePreInit```

      * ```Luanch.cpp GEngineloop.PreInit``` -> ```LuanchEngineLoop.cpp FEngineLoop::PreInit```

      * ```LuanchEngineLoop.cpp FEngineLoop::PreInitPreStartupScreen```

        * ```GLog->SetCurrentThreadAsMasterThread()``` 日志 ?

        * ```FMemory::SetupTLSCachesOnCurrentThread()``` 内存 ?

        * ```FPlatformProcess::SetCurrentWorkingDirectoryToBaseDir()``` 转换到可执行文件的文件夹下面

        * ```GLog->EnableBacklog(true)```

        * ```BeginPreInitTextLocalization()``` 这是什么

        * 初始化随机数

        * ```c++
          // 创建 task graph 和线程池
          FTaskGraphInterface::Startup(FPlatformMisc::NumberOfCores());
          FTaskGraphInterface::Get().AttachToThread(ENamedThreads::GameThread);
          ```

        * ```LoadCoreModules()``` 加载运行需要的必要模块, 待深入

        * ```GLargeThreadPool->Create(NumThreadsInLargeThreadPool, StackSize * 1024)``` 如果过在编辑器模式下, 创建一个较大的线程池, 用于构建光照等

        * ```LoadPreInitModules()```

        * ```c++
          FConfigCacheIni::LoadConsoleVariablesFromINI()
          FPlatformMisc::PlatformInit()
          FPlatformMemory::Init()
          // 杂项`
          ```

        * ```FSlateApplication::Create()``` 老朋友

        * ```c++
          FShaderParametersMetadata::InitializeAllUniformBufferStructs()
          RHIInit(bHasEditorToken)
          RenderUtilsInit()
          FShaderCodeLibrary::InitForRuntime(GMaxRHIShaderPlatform)
          FShaderPipelineCache::Initialize(GMaxRHIShaderPlatform)
          ```

          ! rendering 相关, 初始化 RHI, RenderUtil

        * ```c++
          GShaderCompilerStats = new FShaderCompilerStats();
          
          check(!GShaderCompilingManager);
          GShaderCompilingManager = new FShaderCompilingManager();
          
          check(!GDistanceFieldAsyncQueue);
          GDistanceFieldAsyncQueue = new FDistanceFieldAsyncQueue();
          
          // Shader hash cache is required only for shader compilation.
          InitializeShaderHashCache();
          ```

          shader编译 相关

        * ```c++
          GetRendererModule() // 主线程中缓存渲染模块, 用于渲染线程后续的提取
          ```

        * ```c++
          InitializeShaderTypes() // 在加载所有 shader 之前初始化类型
          ```

        * ```c++
          CompileGlobalShaderMap(false); // 编译全局 shader, 注意 shader 的编译十分之早
          ```
        
        * ```c++
          PostInitRHI()
          ```
        
        * ```c++
          StartRenderingThread() // ! 启动渲染线程
          ```
        
        * ```c++
          FSlateApplication& CurrentSlateApp = FSlateApplication::Get();
          CurrentSlateApp.InitializeRenderer(SlateRendererSharedRef); // 初始化 slate 渲染器, UI 相关
          ```
        
        * 保存 ```PreInitContext ```
        
      * ```LuanchEngineLoop.cpp FEngineLoop::PreInitPostStartupScreen``` 注意 preStart 和 postStart 的不同, 虽然都是在```FEngineLoop::PreInit``` 中被调用
      
        * 恢复 ```PreInitContext```
        * 
    
  * ```LaunchWindows.cpp: GuardedMain``` 

    * ```EditorInit(GEngineLoop)``` 编辑器初始化, 暂时不看

    * ```EngineInit() ``` 

      * ```GEngineLoop.Init()``` -> ```LaunchEngineLoop.cpp FEngineLoop::Init()```

        * ```c++
          		FString UnrealEdEngineClassName;
            		GConfig->GetString(TEXT("/Script/Engine.Engine"), TEXT("UnrealEdEngine"), UnrealEdEngineClassName, GEngineIni);
            		EngineClass = StaticLoadClass(UUnrealEdEngine::StaticClass(), nullptr, *UnrealEdEngineClassName);
            		if (EngineClass == nullptr)
            		{
            			UE_LOG(LogInit, Fatal, TEXT("Failed to load UnrealEd Engine class '%s'."), *UnrealEdEngineClassName);
            		}
            		GEngine = GEditor = GUnrealEd = NewObject<UUnrealEdEngine>(GetTransientPackage(), EngineClass); // 此处为编辑器模式, 非编辑器模式下只有 GEngine 会被初始化
          ```

          	创建引擎类
          	
        * ```GEngine->Init(this)``` 调用引擎类的初始化函数

          * 加载插件
          * ```AddToRoot``` 避免 GC, 因为 UEngine 也继承了 UObject, 但同时又继承了 FExec, 这是因为 FExec 不继承自 UObject 吗
          * ```FCoreUObjectDelegates::GetPreGarbageCollectDelegate().AddStatic(UEngine::PreGarbageCollect);``` 这是啥东东

          * ```UGameEngine::Init```
            * 创建游戏实例 ```NewObject``` 方法
            * ```instance->initializestandalone()``` 
            * 创建 ```viewportClient```
            * 创建 ```GameViewport```

        * ```GEngine->Start()```

          * 啥都没有 ???
          * ```UGameEngine::Start()``` 
            * ```GameInstance->StartGameInstance()``` 启动游戏实例
            * ```GameInstance->OnStart()``` 接下来就是 gameplay 相关了, 暂时跳过

        * ```c++
          FModuleManager::Get().LoadModule("...") // 加载一些模块
          ```

        * ```c++
          FViewport::SetGameRenderingEnabled(true, 3)
          ```

        * ```c++
          FThreadHeartBeat::Get().Start()
          ```

    * ```DumpBootTiming()```

### 循环更新流程

* ```LaunchWindows.cpp: GuardedMain``` 
  * ```EngineTick()```

### 退出流程

* ```LaunchWindows.cpp: GuardedMain``` 
  * ```EditorExit()```
* ```LaunchWindows.cpp WinMain```
  * ```FEngineLoop::AppExit()```









