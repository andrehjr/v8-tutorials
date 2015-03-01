/*
  Exposing properties from objects in v8. 
  Usage: ./out/expose-objects game.js

*/

#include <iostream>
#include "include/v8.h"
#include "include/libplatform/libplatform.h"
#include "common/common.h"

using namespace v8;
using namespace std;

//Here will be a simple game class, with one method
//that we will expose to scripts. This is a direct function
//being exposed, under a 'c++ game class', in scripts. 
//In other words - the script side game object - will actually
//represent an instance of this class. What it doesn't mean -
//that game is now a type! This is a common catch - you are not
//exposing a type, simply injecting a global object. Once
//you injected that object - you injected a function into that
//objects property set. Imagine the following javascript instead :

//  var game = {}; 
//      game.start = function() { print('game started!'); }


//Declare these in advance
class Game;
Game* UnwrapGameObject(Local<Object> jsObject );

class Game {

public:
    Game() { }
    ~Game() { }
private:
    //The direct function of this class 
    //that will get called on.
    void start() 
    {
        printf("Game started!\n");
    }

    //The v8 handler for this specific function.
    //Of course, you could just printf() inside here
    //but the point is to demonstrate unwrapping a
    //native c++ pointer from a javascript callback.
public:
    static void gameStart(const FunctionCallbackInfo<Value>& args)
    {
        HandleScope scope(args.GetIsolate());

        //So , we can unwrap a c++ instance by using the args.Holder() or args.This()
        //This usually happens when the user has game.start(); the 'game' is what args
        //stores for us to use and unwrap.
        Game* game = UnwrapGameObject( args.Holder() );
        game->start();
    }
};


//Here is a helper function to ease the process - This inserts a named property with a callback
//into the object requested. For example, game.start <- this would be simpler using the function here
void ExposeProperty(Isolate* isolate, Local<Object> intoObject, const char* name, FunctionCallback callback)
{
    HandleScope handle_scope(isolate);
    Local<Function> fn = FunctionTemplate::New(isolate, callback)->GetFunction();
    Local<String> fn_name = String::NewFromUtf8(isolate, name);
    fn->SetName(fn_name);
    intoObject->Set(fn_name, fn);
}

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

//This will expose an object with the type Game, into the global scope.
//It will return a handle to the JS object that represents this c++ instance.
Handle<Object> WrapGameObject(Isolate* isolate, Game *gameInstance )
{
    //This will persist when we leave the handle scope,
    //meaning that the actual c++ instance is preserved
    //This is just the preset for an emtpy object
    Local<ObjectTemplate> raw_template = ObjectTemplate::New(isolate);

    //This is so we can store a c++ object on it
	raw_template->SetInternalFieldCount(1);

    //Create the actual template object, 
    //class_template = Persistent<ObjectTemplate>::New(isolate, raw_template);    
    //Create an instance of the js object 
	Handle<Object> result = raw_template->NewInstance();

    //Create a wrapper for the c++ instance
    //Store the 'external c++ pointer' inside the JS object
	result->SetInternalField(0 , External::New(isolate, gameInstance));
    //Return the JS object representing this class
	return result;
}

//This will return the c++ object that WrapGameObject stored, 
//from an existing jsObject. Used in the start callback
Game* UnwrapGameObject(Local<Object> jsObject ) 
{
    Local<External> pointer = Local<External>::Cast( jsObject->GetInternalField(0) );
    return static_cast<Game*>( pointer->Value() );
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

        Local<ObjectTemplate> global = ObjectTemplate::New(isolate);

        //The most obvious form of getting values into script - set a value on the global scope
        //Note that i set the flags to ReadOnly. This prevents 'version = 5;' from overriding
        //this object accidentally or intentionally.
        global->Set(String::NewFromUtf8(isolate, "version"), String::NewFromUtf8(isolate, "1.1"));

        //add the print function to the scope too
        global->Set(String::NewFromUtf8(isolate, "print"), v8::FunctionTemplate::New(isolate, printMessage ));

        //Create our main context, we need to enter it
        Handle<Context> context = Context::New(isolate, NULL, global);

        //Last example had no need to, as it was not creating anything.
        //If we create a JS object, we need to be in a context.
        //Creating a context scope says that anything within the scope
        //of this - will be created inside the context specified. If
        //you get errors at runtime with  	
            //v8::internal::Top::global_context() 
        //on top of the stack - you are trying to create objects in
        //a non existant context. Enter a context, and then create some.

        Context::Scope context_scope( context );

        //Now that we have a context - we can start injecting stuff into it.
        Game* game = new Game();

        //We will pack this object into a JS object c++ side, so that we can
        //manipulate the script version from here.
        Handle<Object> jsGame = WrapGameObject(isolate, game);

        //Now, we can easily expose the start function for js to use
        ExposeProperty(isolate, jsGame, "start", FunctionCallback( Game::gameStart ) );

        //And finally, add the game object with its functions, into the global scope
        context->Global()->Set( String::NewFromUtf8(isolate, "game"), jsGame);

        if(argc > 1) {
            eScriptExecResult r = executeScript(isolate, context, string( argv[1] ));
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