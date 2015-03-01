/*
    Exposing types into js with v8. 
    Usage: ./out/expose-types point.js
*/

#include <iostream>
#include "include/v8.h"
#include "include/libplatform/libplatform.h"
#include "common/common.h"

#include "vec2.h"

using namespace v8;
using namespace std;

class Point {
    public:
        Point(int x, int y) : x_(x), y_(y) { }
        int x_, y_;
};

static void PointConstructor(const FunctionCallbackInfo<Value>& args)
{
    HandleScope scope(args.GetIsolate());
    Isolate* isolate = args.GetIsolate();

    int x = 0;
    int y = 0;

    if (!args[0].IsEmpty() && args[0]->IsNumber())
    {
        x = args[0]->NumberValue();
    }

    if (!args[1].IsEmpty() && args[1]->IsNumber())
    {
        y = args[1]->NumberValue();
    }

    Point* point = new Point(x, y);
    args.This()->SetInternalField(0, External::New(isolate, point));
}

static void GetPointX(Local<String> property,
             const PropertyCallbackInfo<Value>& info) {
    Local<Object> self = info.Holder();
    Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
    void* ptr = wrap->Value();
    int value = static_cast<Point*>(ptr)->x_;
    info.GetReturnValue().Set(value);
}

static void SetPointX(Local<String> property, Local<Value> value,
             const PropertyCallbackInfo<void>& info) {
    Local<Object> self = info.Holder();
    Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
    void* ptr = wrap->Value();
    static_cast<Point*>(ptr)->x_ = value->Int32Value();
}

static void GetPointY(Local<String> property,
             const PropertyCallbackInfo<Value>& info) {
    Local<Object> self = info.Holder();
    Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
    void* ptr = wrap->Value();
    int value = static_cast<Point*>(ptr)->y_;
    info.GetReturnValue().Set(value);
}

static void SetPointY(Local<String> property, Local<Value> value,
             const PropertyCallbackInfo<void>& info) {
    Local<Object> self = info.Holder();
    Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
    void* ptr = wrap->Value();
    static_cast<Point*>(ptr)->x_ = value->Int32Value();
}

static void exposePoint(Isolate* isolate, Handle<ObjectTemplate> context) {
    HandleScope scope(isolate);

    Local<FunctionTemplate> point_templ = FunctionTemplate::New(isolate, PointConstructor);
    Local<ObjectTemplate> obj = point_templ->InstanceTemplate();
    obj->SetInternalFieldCount(1);

    // Set accessors
    obj->SetAccessor(String::NewFromUtf8(isolate, "x"), GetPointX, SetPointX);
    obj->SetAccessor(String::NewFromUtf8(isolate, "y"), GetPointY, SetPointY);

    // Register constructor
    context->Set(String::NewFromUtf8(isolate, "Point"), point_templ);
 }

/* a simple print function, for printing information to stdout */
static void printMessage(const FunctionCallbackInfo<Value>& args)
{
    HandleScope scope(args.GetIsolate());

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

        Local<ObjectTemplate> global = ObjectTemplate::New();

        //add the print function to the scope too
        global->Set(isolate, "print", FunctionTemplate::New(isolate, printMessage ));

        //Now, we expose (before the context) the type into the template.
        exposePoint(isolate, global);

        //Create our main context, we don't need to enter it now
        Handle<Context> context = Context::New(isolate, NULL, global);

        printf("executeScript\n");
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