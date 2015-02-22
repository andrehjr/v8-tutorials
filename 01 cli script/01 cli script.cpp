/*

01 - Command line script execution
    
    A simple primer to understanding the v8 workflow. This is not meant
    to do much more than print a result of a script execution. But, it 
    does demonstrate how to create a global template, a context, and 
    how the script file makes it's way from disk into the context.
    
    The scope of tutorial 1 is not to dig into those details, but a method
    of getting v8 running within minutes for you to mess around with.

Common: 
    What?
        A simple introduction to understanding the v8 javascript engine.
    Why?
        Written for the #altdevblog www.altdevblogaday.com group.
    Who? 
        Written by Sven Bergström ( FuzzYspo0N ).

    
Discussion :
    
    These examples will serve the simple purpose of demonstrating commonly 
    asked questions with regards to v8 and the embedding of javascript 
    into a c++ application.

    Disclaimer - I am no expert on v8. I just use it, and have hit the wall
    a number of times. Hopefully, anything I say will help you understand the
    mentality, concepts and structure of the v8 engine and how to use it based
    on the journey that I have travelled with the engine.

    For the full post and more info - see the following link -


*/

#include <iostream>
#include "include/v8.h"
#include "include/libplatform/libplatform.h"
#include "../common/common.h"
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

        Locker locker(isolate);

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