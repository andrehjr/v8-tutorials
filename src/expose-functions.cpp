/*

    02 - Exposing global functions in v8.
        
        Verbosity is a nightmare when scripting, and often the most used
        functions should be named and easily accessible for scripters. 
        For example, the print/echo/log/etc function should be a globally
        accessible function with a short and easy to remember name. 

    This tutorial covers exposing functions directly as global functions.

Common: 
    What?
        A simple introduction to understanding the v8 javascript engine.
    Why?
        Written for the #altdevblog www.altdevblogaday.com group.
    Who? 
        Written by Sven Bergstr�m ( FuzzYspo0N ).

    
Discussion :
    
    These examples will serve the simple purpose of demonstrating commonly 
    asked questions with regards to v8 and the embedding of javascript 
    into a c++ application.

    Disclaimer - I am no expert on v8. I just use it, and have hit the wall
    a number of times. Hopefully, anything I say will help you understand the
    mentality, concepts and structure of the v8 engine and how to use it based
    on the journey that I have travelled with the engine.

    For the full post and more info - see the following link - http://altdevblogaday.com/change-is-as-good-as-a-holiday-an-introductio

*/

#include <iostream>
#include "include/libplatform/libplatform.h"
#include "common/common.h"

using namespace v8;
using namespace std;

    /* a simple print function, for printing information to stdout */
static void printMessage(const FunctionCallbackInfo<Value>& args)
{
    HandleScope scope(args.GetIsolate());

    //The arguments that are handed in have some valuable information
    //tucked away inside it. Such as the function or object that it was
    //called from (in this case, global) and also can be a variable length.
    //Most times, you can access the arguments directly as args[ index ].
    //Also note how you can use To(Type)->(Type)Value() to get the value.
    //For example, args[1]->ToBoolean()->BooleanValue();

    if( args.Length() > 0) {
        String::Utf8Value value(args[0]->ToString());

        if( value.length() > 0) 
        {
            //Print the message to stdout
            printf("From v8: %s\n", *value);
        }
    }
}

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

        //Here, we can now expose the printMessage function. Note that it is
        //done before the context is created (obviously, for it to exist in that context).
        //This creates a new global function object called print();

        Local<ObjectTemplate> global = ObjectTemplate::New(isolate);
        global->Set(String::NewFromUtf8(isolate, "print"), FunctionTemplate::New(isolate, printMessage));

        //Create our main context
        Handle<Context> context = Context::New(isolate, NULL, global);

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