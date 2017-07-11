#include "Properties.h"
#include "Data.h"

namespace osn {

Nan::Persistent<v8::FunctionTemplate> Properties::prototype = 
    Nan::Persistent<v8::FunctionTemplate>();

Properties::Properties(obs::properties &&handle)
 : handle(std::move(handle))
{
}

Properties::Properties(std::string id, obs::properties::object_type type)
 : handle(id, type)
{
}

NAN_MODULE_INIT(Properties::Init)
{
    auto locProto = Nan::New<v8::FunctionTemplate>(New);
    locProto->SetClassName(FIELD_NAME("Properties"));
    locProto->InstanceTemplate()->SetInternalFieldCount(1);

    Nan::SetMethod(locProto->PrototypeTemplate(), "first", first);
    Nan::SetMethod(locProto->PrototypeTemplate(), "count", count);
    Nan::SetMethod(locProto->PrototypeTemplate(), "get", get);

    Nan::Set(target, FIELD_NAME("Properties"), locProto->GetFunction());
    prototype.Reset(locProto);
}

NAN_METHOD(Properties::New)
{
    if (!info.IsConstructCall()) {
        Nan::ThrowError("Must be used as a construct call");
        return;
    }

    if (info.Length() != 2) {
        Nan::ThrowError("Unexpected number of arguments");
        return;
    }

    if (!info[0]->IsString()) {
        Nan::ThrowError("Invalid type passed - expected string");
        return;
    }

    if (!info[1]->IsUint32()) {
        Nan::ThrowError("Invalid type passed - expected integer");
        return;
    }
    
    Nan::Utf8String id(info[0]);
    uint32_t object_type = Nan::To<uint32_t>(info[1]).FromJust();
    Properties *object = new Properties(*id, static_cast<obs::properties::object_type>(object_type));
    object->Wrap(info.This());
    info.GetReturnValue().Set(info.This());
}

NAN_GETTER(Properties::status)
{
    obs::properties &handle = Properties::Object::GetHandle(info.Holder());

    info.GetReturnValue().Set(handle.status());
}

NAN_METHOD(Properties::first)
{
    obs::properties &handle = Properties::Object::GetHandle(info.Holder());

    Property *binding = new Property(handle.first());
    auto object = Property::Object::GenerateObject(binding);

    info.GetReturnValue().Set(object);
}

NAN_METHOD(Properties::count)
{
    obs::properties &handle = Properties::Object::GetHandle(info.Holder());

    obs::property it = handle.first();

    int result = 0;

    while (it.status() == obs::property::status_type::okay) {
        ++result;
    }

    info.GetReturnValue().Set(result);
}

NAN_METHOD(Properties::get)
{
    obs::properties &handle = Properties::Object::GetHandle(info.Holder());

    if (info.Length() != 1 || !info[0]->IsString()) {
        Nan::ThrowError("Unexpected arguments");
        return;
    }

    Nan::Utf8String property_name(info[0]);
    
    Property *binding = new Property(handle.get(*property_name));
    auto object = Property::Object::GenerateObject(binding);

    info.GetReturnValue().Set(object);
}

NAN_METHOD(Properties::apply)
{
    /* TODO */
}

Nan::Persistent<v8::FunctionTemplate> Property::prototype =
    Nan::Persistent<v8::FunctionTemplate>();

Property::Property(obs::property &property)
 : handle(property)
{
}

NAN_MODULE_INIT(Property::Init)
{
    auto locProto = Nan::New<v8::FunctionTemplate>();
    locProto->SetClassName(FIELD_NAME("Property"));
    locProto->InstanceTemplate()->SetInternalFieldCount(1);

    /* iterable */
    Nan::SetAccessor(locProto->InstanceTemplate(), FIELD_NAME("done"), done);
    Nan::SetAccessor(locProto->InstanceTemplate(), FIELD_NAME("value"), value);

    Nan::SetAccessor(locProto->InstanceTemplate(), FIELD_NAME("name"), name);
    Nan::SetAccessor(locProto->InstanceTemplate(), FIELD_NAME("type"), type);
    Nan::SetAccessor(locProto->InstanceTemplate(), FIELD_NAME("status"), status);
    Nan::SetAccessor(locProto->InstanceTemplate(), FIELD_NAME("description"), description);
    Nan::SetAccessor(locProto->InstanceTemplate(), FIELD_NAME("longDescription"), longDescription);
    Nan::SetAccessor(locProto->InstanceTemplate(), FIELD_NAME("details"), details);
    Nan::SetAccessor(locProto->InstanceTemplate(), FIELD_NAME("enabled"), enabled);
    Nan::SetAccessor(locProto->InstanceTemplate(), FIELD_NAME("visible"), visible);
    Nan::SetMethod(locProto->PrototypeTemplate(), "next", next);

    Nan::Set(target, FIELD_NAME("Property"), locProto->GetFunction());
    prototype.Reset(locProto);
}

NAN_GETTER(Property::status)
{
    obs::property &handle = Property::Object::GetHandle(info.Holder());

    info.GetReturnValue().Set(handle.status());
}

NAN_GETTER(Property::name)
{
    obs::property &handle = Property::Object::GetHandle(info.Holder());

    info.GetReturnValue().Set(Nan::New(handle.name().c_str()).ToLocalChecked());
}

NAN_GETTER(Property::description)
{
    obs::property &handle = Property::Object::GetHandle(info.Holder());

    info.GetReturnValue().Set(Nan::New(handle.description().c_str()).ToLocalChecked());
}

NAN_GETTER(Property::longDescription)
{
    obs::property &handle = Property::Object::GetHandle(info.Holder());

    info.GetReturnValue().Set(Nan::New(handle.long_description().c_str()).ToLocalChecked());
}

NAN_GETTER(Property::type)
{
    obs::property &handle = Property::Object::GetHandle(info.Holder());

    info.GetReturnValue().Set(handle.type());
}

NAN_GETTER(Property::enabled)
{
    obs::property &handle = Property::Object::GetHandle(info.Holder());

    info.GetReturnValue().Set(handle.enabled());
}

NAN_GETTER(Property::visible)
{
    obs::property &handle = Property::Object::GetHandle(info.Holder());

    info.GetReturnValue().Set(handle.visible());
}

NAN_GETTER(Property::value)
{
    info.GetReturnValue().Set(info.Holder());
}

NAN_GETTER(Property::done)
{
    obs::property &handle = Property::Object::GetHandle(info.Holder());

    info.GetReturnValue().Set(
        Nan::New<v8::Boolean>(
            handle.status() != obs::property::status_type::okay));
}

NAN_METHOD(Property::next)
{
    obs::property &handle = Property::Object::GetHandle(info.Holder());

    handle.next();

    info.GetReturnValue().Set(info.Holder());
}

namespace {

v8::Local<v8::Array> GetListItems(obs::list_property &property)
{
    int count = static_cast<int>(property.count());
    auto array = Nan::New<v8::Array>(count);

    for (int i = 0; i < count; ++i) {
        auto object = Nan::New<v8::Object>();

        Nan::Set(object, 
            FIELD_NAME("name"), 
            Nan::New<v8::String>(property.get_name(i)).ToLocalChecked());

        switch (property.format()) {
        case OBS_COMBO_FORMAT_INT:
            Nan::Set(object, 
                FIELD_NAME("value"), 
                Nan::New<v8::Integer>(static_cast<int>(property.get_integer(i))));
            break;
        case OBS_COMBO_FORMAT_STRING:
            Nan::Set(object, 
                FIELD_NAME("value"), 
                Nan::New<v8::String>(property.get_string(i)).ToLocalChecked());
            break;
        case OBS_COMBO_FORMAT_FLOAT:
            Nan::Set(object, 
                FIELD_NAME("value"), 
                Nan::New<v8::Number>(property.get_float(i)));
            break;
        }

        Nan::Set(array, i, object);
    }

    return array;
}

}

NAN_GETTER(Property::details)
{
    obs::property &handle = Property::Object::GetHandle(info.Holder());

    /* Some types have extra data. Associated with them. Construct objects
       and pass them with the property since you're more likely to need them
       than not. */
    auto object = Nan::New<v8::Object>();

    switch (handle.type()) {
    case OBS_PROPERTY_LIST: {
        obs::list_property list_prop = 
            handle.list_property();

        Nan::Set(object, 
            FIELD_NAME("format"), 
            Nan::New<v8::Integer>(list_prop.format()));

        Nan::Set(object, 
            FIELD_NAME("items"), 
            GetListItems(list_prop));
        break;
    }
    case OBS_PROPERTY_EDITABLE_LIST: {
        obs::editable_list_property edit_list_prop = 
            handle.editable_list_property();

        Nan::Set(object, 
            FIELD_NAME("type"), 
            Nan::New<v8::Integer>(edit_list_prop.type()));

        Nan::Set(object, 
            FIELD_NAME("format"), 
            Nan::New<v8::Integer>(edit_list_prop.format()));

        Nan::Set(object, 
            FIELD_NAME("items"), 
            GetListItems(static_cast<obs::list_property>(edit_list_prop)));

        Nan::Set(object, 
            FIELD_NAME("filter"), 
            Nan::New<v8::String>(edit_list_prop.filter()).ToLocalChecked());

        Nan::Set(object, 
            FIELD_NAME("defaultPath"), 
            Nan::New<v8::String>(edit_list_prop.default_path()).ToLocalChecked());
        break;
    }
    case OBS_PROPERTY_TEXT:  {
        obs::text_property text_prop = handle.text_property();

        Nan::Set(object, 
            FIELD_NAME("type"), 
            Nan::New<v8::Integer>(text_prop.type()));
        break;
    }
    case OBS_PROPERTY_PATH: {
        obs::path_property path_prop = handle.path_property();

        Nan::Set(object, 
            FIELD_NAME("type"), 
            Nan::New<v8::Integer>(path_prop.type()));
        break;
    }
    case OBS_PROPERTY_FLOAT: {
        obs::float_property float_prop = 
            handle.float_property();

        Nan::Set(object, 
            FIELD_NAME("type"), 
            Nan::New<v8::Integer>(float_prop.type()));
        break;
    }
    case OBS_PROPERTY_INT: {
        obs::integer_property int_prop = 
            handle.integer_property();

        Nan::Set(object, 
            FIELD_NAME("type"), 
            Nan::New<v8::Integer>(int_prop.type()));
        break;
    }
    }

    info.GetReturnValue().Set(object);
}

}