#include <node.h>
#include "object_wrap.h"
#include "object_builder.h"
#include <unordered_map>
#include <tuple>
#include <string>

namespace {

struct Element;

using Map = std::unordered_map< std::string, Element >;

struct Element {
	Map &map;
	std::string const key;
	v8::Global<v8::Value> value;

	static void finalizationCb(v8::WeakCallbackInfo<Element> const &data) {
		auto element = data.GetParameter();
		element->map.erase(element->key);
	}

	void set(v8::Isolate *isolate, v8::Local<v8::Value> handle) {
		value.Reset(isolate, handle);
		value.SetWeak(this, finalizationCb, v8::WeakCallbackType::kParameter);
	}

	Element(Map &map, std::string key) : map{map}, key{key} {};
};

}  // anonymous namespace

class WeakValueMap : public ObjectWrap {
public:
	static void Init( v8::Local<v8::Object> _exports ) {
		ObjectBuilder exports { v8::Isolate::GetCurrent(), _exports };

		exports.add_class( "WeakValueMap", constructor, []( ClassBuilder cls ) {
			cls.set_internal_field_count( 1 );
			cls.add_property( "size", size_getter );
			cls.add_method( "get",    wrap<&WeakValueMap::get_method> );
			cls.add_method( "set",    wrap<&WeakValueMap::set_method> );
			cls.add_method( "delete", wrap<&WeakValueMap::delete_method> );
			cls.add_method( "_keysAsArray", keys_as_array );
		});
	}

private:
	Map map;

	using Args = v8::FunctionCallbackInfo<v8::Value> const;

	static void constructor( Args &args ) {
		auto isolate = args.GetIsolate();

		//Make sure this is a new-call or throw a type error
		if (!args.IsConstructCall()) {
			auto msg = v8::String::NewFromUtf8(isolate, "Constructor WeakValueMap requires 'new'", v8::NewStringType::kNormal).ToLocalChecked();
			isolate->ThrowException(v8::Exception::TypeError(msg));
			return;
		}

		auto obj = new WeakValueMap();
		obj->Wrap(args.This());
		args.GetReturnValue().Set(args.This());
	}

	static void size_getter( v8::Local<v8::String>, v8::PropertyCallbackInfo<v8::Value> const &info ) {
		auto obj = ObjectWrap::Unwrap<WeakValueMap>( info.Holder() );
		info.GetReturnValue().Set( (uint32_t) obj->map.size() );
	}

	using method_t = void (WeakValueMap::*)( Args &args, std::string const &key );

	template< method_t method >
	static void wrap( Args &args ) {
		auto obj = ObjectWrap::Unwrap<WeakValueMap>( args.Holder() );
		auto isolate = args.GetIsolate();
		auto &&key = v8::String::Utf8Value{ args.GetIsolate(), args[0]->ToString(isolate->GetCurrentContext()).ToLocalChecked() };
		if( *key )
			(obj->*method)( args, *key );
	}

	void get_method( Args &args, std::string const &key ) {
		auto i = map.find( key );
		if( i != map.end() )
			args.GetReturnValue().Set( i->second.value );
	}

	void set_method( Args &args, std::string const &key ) {
		//Delete from the map if the value to insert is undefined
		if( args[1]->IsUndefined() )
			return delete_method( args, key );

		auto i = map.emplace( std::piecewise_construct,
				std::forward_as_tuple( key ),
				std::forward_as_tuple( map, key ) ).first;
		i->second.set( args.GetIsolate(), args[1] );

		args.GetReturnValue().Set( args.This() );
	}

	void delete_method( Args &args, std::string const &key ) {
		map.erase( key );

		args.GetReturnValue().Set( args.This() );
	}

	static void keys_as_array( Args &args) {
		auto isolate = args.GetIsolate();
		auto obj = ObjectWrap::Unwrap<WeakValueMap>(args.Holder());

		v8::Local<v8::Array> keysArray = v8::Array::New(isolate, obj->map.size());

		int i = 0;
		for (auto entry = obj->map.begin(); entry != obj->map.end(); ++entry) {
			keysArray->Set(isolate->GetCurrentContext(), i++, v8::String::NewFromUtf8(isolate, entry->first.data(), v8::NewStringType::kNormal).ToLocalChecked());
		}
		args.GetReturnValue().Set(keysArray);
	}
};

NODE_MODULE(addon, WeakValueMap::Init)
