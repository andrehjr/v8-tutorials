/*
    Just run any file passed from args
    Usage: ./out/cli-script anyfile.js
*/

#include <iostream>
#include "include/v8.h"
#include "include/libplatform/libplatform.h"
#include "common/common.h"
using namespace v8;
using namespace std;

int main(int argc, char **argv) 
{
    // Initialize V8.
    V8::InitializeICU();
    Platform* platform = platform::CreateDefaultPlatform();
    V8::InitializePlatform(platform);
    V8::Initialize();

    //Quite simple, these are scope based 'managers'.
    //One protects threading issues, and one manages
    //the creation of JS handles, for clean up.

    Isolate* isolate = Isolate::New();
    {
        Isolate::Scope isolate_scope(isolate);

        // Create a stack-allocated handle scope.
        HandleScope handle_scope(isolate);

        // Create a new context.
        Local<Context> context = Context::New(isolate);

        //A context is a fresh execution context. Take for example, 
        //a browser tab or frame in Google Chrome. Each of these are
        //a new context, stamped with a global template, and created
        //on demand. When executing scripts, you can execute them in 
        //and existing context by using the Context::context_scope 
        //handlers. This 'switches' context for that execution period
        Context::Scope context_scope(context);

        //If we receive an argument, execute the script file

        if(argc > 1) {
            eScriptExecResult r = executeScript(isolate, context, string(argv[1]));
        } else {
            printf("Usage: <scriptname.js> \n Execute the javascript file.");
        }

    }

    // Dispose the isolate and tear down V8.
    isolate->Dispose();
    V8::Dispose();
    V8::ShutdownPlatform();
    delete platform;
    return 0;
}