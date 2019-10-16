#include <cstring>
#include <vector>
#include <iostream>
#include <map>

#include "webgl.h"
#include "image.h"
#include <node.h>
#include <node_buffer.h>
#include <GL/glew.h>

#ifdef _WIN32
  #define  strcasestr(s, t) strstr(strupr(s), strupr(t))
#endif

#define CHECK_ARRAY_BUFFER(val) if(!val->IsArrayBufferView()) \
        {Nan::ThrowTypeError("Only support array buffer"); return;}

namespace webgl {

using namespace node;
using namespace v8;
using namespace std;

// forward declarations
enum GLObjectType {
  GLOBJECT_TYPE_BUFFER,
  GLOBJECT_TYPE_FRAMEBUFFER,
  GLOBJECT_TYPE_PROGRAM,
  GLOBJECT_TYPE_RENDERBUFFER,
  GLOBJECT_TYPE_SHADER,
  GLOBJECT_TYPE_TEXTURE,
  GLOBJECT_TYPE_SAMPLER,
  GLOBJECT_TYPE_TRANSFORM_FEEDBACK,
};

void registerGLObj(GLObjectType type, GLuint obj);
void unregisterGLObj(GLuint obj);
int registerSync(GLsync);
void unregisterSync(int syncId);
GLsync getSync(int syncId);

// A 32-bit and 64-bit compatible way of converting a pointer to a GLuint.
static GLuint ToGLuint(const void* ptr) {
  return static_cast<GLuint>(reinterpret_cast<size_t>(ptr));
}

template<typename Type>
inline Type* getArrayData(Local<Value> arg, int* num = NULL) {
  Type *data=NULL;
  if(num) *num=0;

  if(!arg->IsNull()) {
    if(arg->IsArray()) {
      Nan::ThrowError("Not supported: array type (use typed array)");
      /*
      Local<Array> arr = Local<Array>::Cast(arg);
      if(num) *num=arr->Length();
      data = reinterpret_cast<Type*>(arr->GetIndexedPropertiesExternalArrayData());*/
    }
    else if(arg->IsObject()) {
      Local<ArrayBufferView> arr = Local<ArrayBufferView>::Cast(arg);
      if(num) *num=arr->ByteLength()/sizeof(Type);
      data = reinterpret_cast<Type*>((uint8_t*)arr->Buffer()->GetContents().Data() + arr->ByteOffset());
    }
    else
      Nan::ThrowError("Bad array argument");
  }

  return data;
}

inline void* getImageData(Local<Value> arg, int& byteSize) {
  void *pixels = NULL;
  if (!arg->IsNull()) {
    Local<Object> obj = Local<Object>::Cast(arg);
    if (!obj->IsObject()){
      Nan::ThrowError("Bad texture argument");
    }else if(obj->IsArrayBufferView()){
        int num;
        
        pixels = getArrayData<BYTE>(obj, &num);
        Local<ArrayBufferView> arr = Local<ArrayBufferView>::Cast(obj);
        byteSize = arr->ByteLength();
    }else{
        pixels = node::Buffer::Data(Nan::Get(obj, JS_STR("data")).ToLocalChecked());
        Nan::ThrowError("TODO: populate byteSize param");
    }
  }
  return pixels;
}

NAN_METHOD(Init) {
  Nan::HandleScope scope;
  GLenum err = glewInit();
  if (GLEW_OK != err)
  {
    /* Problem: glewInit failed, something is seriously wrong. */
    fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
    info.GetReturnValue().Set(JS_INT(-1));
  }else{
    //fprintf(stdout, "Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));

    glEnable(GL_PROGRAM_POINT_SIZE);
    info.GetReturnValue().Set(JS_INT(0));  
  } 
  
}

NAN_METHOD(Uniform1f) {
  Nan::HandleScope scope;

  int location = Nan::To<int>(info[0]).FromJust();
  float x = (float) Nan::To<double>(info[1]).FromJust();

  glUniform1f(location, x);
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(Uniform2f) {
  Nan::HandleScope scope;

  int location = Nan::To<int>(info[0]).FromJust();
  float x = (float) Nan::To<double>(info[1]).FromJust();
  float y = (float) Nan::To<double>(info[2]).FromJust();

  glUniform2f(location, x, y);
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(Uniform3f) {
  Nan::HandleScope scope;

  int location = Nan::To<int>(info[0]).FromJust();
  float x = (float) Nan::To<double>(info[1]).FromJust();
  float y = (float) Nan::To<double>(info[2]).FromJust();
  float z = (float) Nan::To<double>(info[3]).FromJust();

  glUniform3f(location, x, y, z);
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(Uniform4f) {
  Nan::HandleScope scope;

  int location = Nan::To<int>(info[0]).FromJust();
  float x = (float) Nan::To<double>(info[1]).FromJust();
  float y = (float) Nan::To<double>(info[2]).FromJust();
  float z = (float) Nan::To<double>(info[3]).FromJust();
  float w = (float) Nan::To<double>(info[4]).FromJust();

  glUniform4f(location, x, y, z, w);
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(Uniform1i) {
  Nan::HandleScope scope;

  int location = Nan::To<int>(info[0]).FromJust();
  int x = Nan::To<int>(info[1]).FromJust();

  glUniform1i(location, x);
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(Uniform2i) {
  Nan::HandleScope scope;

  int location = Nan::To<int>(info[0]).FromJust();
  int x = Nan::To<int>(info[1]).FromJust();
  int y = Nan::To<int>(info[2]).FromJust();

  glUniform2i(location, x, y);
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(Uniform3i) {
  Nan::HandleScope scope;

  int location = Nan::To<int>(info[0]).FromJust();
  int x = Nan::To<int>(info[1]).FromJust();
  int y = Nan::To<int>(info[2]).FromJust();
  int z = Nan::To<int>(info[3]).FromJust();

  glUniform3i(location, x, y, z);
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(Uniform4i) {
  Nan::HandleScope scope;

  int location = Nan::To<int>(info[0]).FromJust();
  int x = Nan::To<int>(info[1]).FromJust();
  int y = Nan::To<int>(info[2]).FromJust();
  int z = Nan::To<int>(info[3]).FromJust();
  int w = Nan::To<int>(info[4]).FromJust();

  glUniform4i(location, x, y, z, w);
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(Uniform1fv) {
  Nan::HandleScope scope;

  int location = Nan::To<int>(info[0]).FromJust();
  int num=0;
  GLfloat *ptr=getArrayData<GLfloat>(info[1],&num);
  glUniform1fv(location, num, ptr);
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(Uniform2fv) {
  Nan::HandleScope scope;

  int location = Nan::To<int>(info[0]).FromJust();
  int num=0;
  GLfloat *ptr=getArrayData<GLfloat>(info[1],&num);
  num /= 2;

  glUniform2fv(location, num, ptr);
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(Uniform3fv) {
  Nan::HandleScope scope;

  int location = Nan::To<int>(info[0]).FromJust();
  int num=0;
  GLfloat *ptr=getArrayData<GLfloat>(info[1],&num);
  num /= 3;

  glUniform3fv(location, num, ptr);
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(Uniform4fv) {
  Nan::HandleScope scope;

  int location = Nan::To<int>(info[0]).FromJust();
  int num=0;
  GLfloat *ptr=getArrayData<GLfloat>(info[1],&num);
  num /= 4;

  glUniform4fv(location, num, ptr);
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(Uniform1iv) {
  Nan::HandleScope scope;

  int location = Nan::To<int>(info[0]).FromJust();
  int num=0;
  GLint *ptr=getArrayData<GLint>(info[1],&num);

  glUniform1iv(location, num, ptr);
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(Uniform2iv) {
  Nan::HandleScope scope;

  int location = Nan::To<int>(info[0]).FromJust();
  int num=0;
  GLint *ptr=getArrayData<GLint>(info[1],&num);
  num /= 2;

  glUniform2iv(location, num, ptr);
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(Uniform3iv) {
  Nan::HandleScope scope;

  int location = Nan::To<int>(info[0]).FromJust();
  int num=0;
  GLint *ptr=getArrayData<GLint>(info[1],&num);
  num /= 3;
  glUniform3iv(location, num, ptr);
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(Uniform4iv) {
  Nan::HandleScope scope;

  int location = Nan::To<int>(info[0]).FromJust();
  int num=0;
  GLint *ptr=getArrayData<GLint>(info[1],&num);
  num /= 4;
  glUniform4iv(location, num, ptr);
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(PixelStorei) {
  Nan::HandleScope scope;

  int pname = Nan::To<int>(info[0]).FromJust();
  int param = Nan::To<int>(info[1]).FromJust();

  glPixelStorei(pname,param);

  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(BindAttribLocation) {
  Nan::HandleScope scope;

  int program = Nan::To<int>(info[0]).FromJust();
  int index = Nan::To<int>(info[1]).FromJust();
  Nan::Utf8String name(info[2]);

  glBindAttribLocation(program, index, *name);

  info.GetReturnValue().Set(Nan::Undefined());
}


NAN_METHOD(GetError) {
  Nan::HandleScope scope;

  info.GetReturnValue().Set(Nan::New<Integer>(glGetError()));
}


NAN_METHOD(DrawArrays) {
  Nan::HandleScope scope;

  int mode = Nan::To<int>(info[0]).FromJust();
  int first = Nan::To<int>(info[1]).FromJust();
  int count = Nan::To<int>(info[2]).FromJust();

  glDrawArrays(mode, first, count);

  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(UniformMatrix2fv) {
  Nan::HandleScope scope;

  GLint location = Nan::To<int>(info[0]).FromJust();
  GLboolean transpose = Nan::To<bool>(info[1]).FromJust();

  GLsizei count=0;
  GLfloat* data=getArrayData<GLfloat>(info[2],&count);

  if (count < 4) {
    Nan::ThrowError("Not enough data for UniformMatrix2fv");
  }else{
    glUniformMatrix2fv(location, count / 4, transpose, data);

    info.GetReturnValue().Set(Nan::Undefined());
  }
}

NAN_METHOD(UniformMatrix3fv) {
  Nan::HandleScope scope;

  GLint location = Nan::To<int>(info[0]).FromJust();
  GLboolean transpose = Nan::To<bool>(info[1]).FromJust();
  GLsizei count=0;
  GLfloat* data=getArrayData<GLfloat>(info[2],&count);

  if (count < 9) {
    Nan::ThrowError("Not enough data for UniformMatrix3fv");
  }else{
    glUniformMatrix3fv(location, count / 9, transpose, data);
    info.GetReturnValue().Set(Nan::Undefined());
  }
}

NAN_METHOD(UniformMatrix4fv) {
  Nan::HandleScope scope;

  GLint location = Nan::To<int>(info[0]).FromJust();
  GLboolean transpose = Nan::To<bool>(info[1]).FromJust();
  GLsizei count=0;
  GLfloat* data=getArrayData<GLfloat>(info[2],&count);

  if (count < 16) {
    Nan::ThrowError("Not enough data for UniformMatrix4fv");
  }else{
    glUniformMatrix4fv(location, count / 16, transpose, data);
    info.GetReturnValue().Set(Nan::Undefined());
  }
}

NAN_METHOD(GenerateMipmap) {
  Nan::HandleScope scope;

  GLint target = Nan::To<int>(info[0]).FromJust();
  glGenerateMipmap(target);

  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(GetAttribLocation) {
  Nan::HandleScope scope;

  int program = Nan::To<int>(info[0]).FromJust();
  Nan::Utf8String name(info[1]);

  info.GetReturnValue().Set(Nan::New<Number>(glGetAttribLocation(program, *name)));
}


NAN_METHOD(DepthFunc) {
  Nan::HandleScope scope;

  glDepthFunc(Nan::To<int>(info[0]).FromJust());

  info.GetReturnValue().Set(Nan::Undefined());
}


NAN_METHOD(Viewport) {
  Nan::HandleScope scope;

  int x = Nan::To<int>(info[0]).FromJust();
  int y = Nan::To<int>(info[1]).FromJust();
  int width = Nan::To<int>(info[2]).FromJust();
  int height = Nan::To<int>(info[3]).FromJust();

  glViewport(x, y, width, height);

  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(CreateShader) {
  Nan::HandleScope scope;

  GLuint shader=glCreateShader(Nan::To<int>(info[0]).FromJust());
  #ifdef LOGGING
  cout<<"createShader "<<shader<<endl;
  #endif
  registerGLObj(GLOBJECT_TYPE_SHADER, shader);
  info.GetReturnValue().Set(Nan::New<Number>(shader));
}


NAN_METHOD(ShaderSource) {
  Nan::HandleScope scope;

  int id = Nan::To<int>(info[0]).FromJust();
  Nan::Utf8String code(info[1]);

  const char* codes[1];
  codes[0] = *code;
  GLint length=code.length();

  glShaderSource  (id, 1, codes, &length);

  info.GetReturnValue().Set(Nan::Undefined());
}


NAN_METHOD(CompileShader) {
  Nan::HandleScope scope;

  glCompileShader(Nan::To<int>(info[0]).FromJust());

  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(FrontFace) {
  Nan::HandleScope scope;

  glFrontFace(Nan::To<int>(info[0]).FromJust());

  info.GetReturnValue().Set(Nan::Undefined());
}


NAN_METHOD(GetShaderParameter) {
  Nan::HandleScope scope;

  int shader = Nan::To<int>(info[0]).FromJust();
  int pname = Nan::To<int>(info[1]).FromJust();
  int value = 0;
  switch (pname) {
  case GL_DELETE_STATUS:
  case GL_COMPILE_STATUS:
    glGetShaderiv(shader, pname, &value);
    info.GetReturnValue().Set(JS_BOOL(static_cast<bool>(value!=0)));
    break;
  case GL_SHADER_TYPE:
    glGetShaderiv(shader, pname, &value);
    info.GetReturnValue().Set(JS_FLOAT(static_cast<unsigned long>(value)));
    break;
  case GL_INFO_LOG_LENGTH:
  case GL_SHADER_SOURCE_LENGTH:
    glGetShaderiv(shader, pname, &value);
    info.GetReturnValue().Set(JS_FLOAT(static_cast<long>(value)));
    break;
  default:
    Nan::ThrowTypeError("GetShaderParameter: Invalid Enum");
  }
  //info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(GetShaderInfoLog) {
  Nan::HandleScope scope;

  int id = Nan::To<int>(info[0]).FromJust();
  int Len = 1024;
  char Error[1024];
  glGetShaderInfoLog(id, 1024, &Len, Error);

  info.GetReturnValue().Set(JS_STR(Error));
}


NAN_METHOD(CreateProgram) {
  Nan::HandleScope scope;

  GLuint program=glCreateProgram();
  #ifdef LOGGING
  cout<<"createProgram "<<program<<endl;
  #endif
  registerGLObj(GLOBJECT_TYPE_PROGRAM, program);
  info.GetReturnValue().Set(Nan::New<Number>(program));
}


NAN_METHOD(AttachShader) {
  Nan::HandleScope scope;

  int program = Nan::To<int>(info[0]).FromJust();
  int shader = Nan::To<int>(info[1]).FromJust();

  glAttachShader(program, shader);

  info.GetReturnValue().Set(Nan::Undefined());
}


NAN_METHOD(LinkProgram) {
  Nan::HandleScope scope;

  glLinkProgram(Nan::To<int>(info[0]).FromJust());

  info.GetReturnValue().Set(Nan::Undefined());
}


NAN_METHOD(GetProgramParameter) {
  Nan::HandleScope scope;

  int program = Nan::To<int>(info[0]).FromJust();
  int pname = Nan::To<int>(info[1]).FromJust();

  int value = 0;
  switch (pname) {
  case GL_DELETE_STATUS:
  case GL_LINK_STATUS:
  case GL_VALIDATE_STATUS:
    glGetProgramiv(program, pname, &value);
    info.GetReturnValue().Set(JS_BOOL(static_cast<bool>(value!=0)));
    break;
  case GL_ATTACHED_SHADERS:
  case GL_ACTIVE_ATTRIBUTES:
  case GL_ACTIVE_UNIFORMS:
    glGetProgramiv(program, pname, &value);
    info.GetReturnValue().Set(JS_FLOAT(static_cast<long>(value)));
    break;
  default:
    Nan::ThrowTypeError("GetProgramParameter: Invalid Enum");
  }
  //info.GetReturnValue().Set(Nan::Undefined());
}


NAN_METHOD(GetUniformLocation) {
  Nan::HandleScope scope;

  int program = Nan::To<int>(info[0]).FromJust();
  Nan::Utf8String name(info[1]);
  
  info.GetReturnValue().Set(JS_INT(glGetUniformLocation(program, *name)));
}


NAN_METHOD(ClearColor) {
  Nan::HandleScope scope;

  float red = (float) Nan::To<double>(info[0]).FromJust();
  float green = (float) Nan::To<double>(info[1]).FromJust();
  float blue = (float) Nan::To<double>(info[2]).FromJust();
  float alpha = (float) Nan::To<double>(info[3]).FromJust();

  glClearColor(red, green, blue, alpha);

  info.GetReturnValue().Set(Nan::Undefined());
}


NAN_METHOD(ClearDepth) {
  Nan::HandleScope scope;

  float depth = (float) Nan::To<double>(info[0]).FromJust();

  glClearDepth(depth);

  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(Disable) {
  Nan::HandleScope scope;

  glDisable(Nan::To<int>(info[0]).FromJust());
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(Enable) {
  Nan::HandleScope scope;

  glEnable(Nan::To<int>(info[0]).FromJust());
  info.GetReturnValue().Set(Nan::Undefined());
}


NAN_METHOD(CreateTexture) {
  Nan::HandleScope scope;

  int typeTarget = Nan::To<int>(info[0]).FromJust();


  GLuint texture;
  glCreateTextures(typeTarget, 1, &texture);
  #ifdef LOGGING
  cout<<"createTexture "<<typeTarget<<" "<<texture<<endl;
  #endif
  registerGLObj(GLOBJECT_TYPE_TEXTURE, texture);
  info.GetReturnValue().Set(Nan::New<Number>(texture));
}


NAN_METHOD(BindTexture) {
  Nan::HandleScope scope;

  int target = Nan::To<int>(info[0]).FromJust();
  int texture = info[1]->IsNull() ? 0 : Nan::To<int>(info[1]).FromJust();

  glBindTexture(target, texture);
  info.GetReturnValue().Set(Nan::Undefined());
}


NAN_METHOD(TexImage2D) {
  Nan::HandleScope scope;

  int target = Nan::To<int>(info[0]).FromJust();
  int level = Nan::To<int>(info[1]).FromJust();
  int internalformat = Nan::To<int>(info[2]).FromJust();
  int width = Nan::To<int>(info[3]).FromJust();
  int height = Nan::To<int>(info[4]).FromJust();
  int border = Nan::To<int>(info[5]).FromJust();
  int format = Nan::To<int>(info[6]).FromJust();
  int type = Nan::To<int>(info[7]).FromJust();
  int dataSize;
  void *pixels=getImageData(info[8], dataSize);

  glTexImage2D(target, level, internalformat, width, height, border, format, type, pixels);

  info.GetReturnValue().Set(Nan::Undefined());
}


NAN_METHOD(TexParameteri) {
  Nan::HandleScope scope;

  int target = Nan::To<int>(info[0]).FromJust();
  int pname = Nan::To<int>(info[1]).FromJust();
  int param = Nan::To<int>(info[2]).FromJust();

  glTexParameteri(target, pname, param);

  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(TexParameterf) {
  Nan::HandleScope scope;

  int target = Nan::To<int>(info[0]).FromJust();
  int pname = Nan::To<int>(info[1]).FromJust();
  float param = (float) Nan::To<double>(info[2]).FromJust();

  glTexParameterf(target, pname, param);

  info.GetReturnValue().Set(Nan::Undefined());
}


NAN_METHOD(Clear) {
  Nan::HandleScope scope;

  glClear(Nan::To<int>(info[0]).FromJust());

  info.GetReturnValue().Set(Nan::Undefined());
}


NAN_METHOD(UseProgram) {
  Nan::HandleScope scope;

  glUseProgram(Nan::To<int>(info[0]).FromJust());

  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(CreateBuffer) {
  Nan::HandleScope scope;

  GLuint buffer;
  glCreateBuffers(1, &buffer);
  #ifdef LOGGING
  cout<<"createBuffer "<<buffer<<endl;
  #endif
  registerGLObj(GLOBJECT_TYPE_BUFFER, buffer);
  info.GetReturnValue().Set(Nan::New<Number>(buffer));
}

NAN_METHOD(BindBuffer) {
  Nan::HandleScope scope;

  int target = Nan::To<int>(info[0]).FromJust();
  int buffer = Nan::To<uint32_t>(info[1]).FromJust();
  glBindBuffer(target,buffer);

  info.GetReturnValue().Set(Nan::Undefined());
}


NAN_METHOD(CreateFramebuffer) {
  Nan::HandleScope scope;

  GLuint buffer;
  glCreateFramebuffers(1, &buffer);
  #ifdef LOGGING
  cout<<"createFrameBuffer "<<buffer<<endl;
  #endif
  registerGLObj(GLOBJECT_TYPE_FRAMEBUFFER, buffer);
  info.GetReturnValue().Set(Nan::New<Number>(buffer));
}


NAN_METHOD(BindFramebuffer) {
  Nan::HandleScope scope;

  int target = Nan::To<int>(info[0]).FromJust();
  int buffer = info[1]->IsNull() ? 0 : Nan::To<int>(info[1]).FromJust();

  glBindFramebuffer(target, buffer);

  info.GetReturnValue().Set(Nan::Undefined());
}


NAN_METHOD(FramebufferTexture2D) {
  Nan::HandleScope scope;

  int target = Nan::To<int>(info[0]).FromJust();
  int attachment = Nan::To<int>(info[1]).FromJust();
  int textarget = Nan::To<int>(info[2]).FromJust();
  int texture = Nan::To<int>(info[3]).FromJust();
  int level = Nan::To<int>(info[4]).FromJust();

  glFramebufferTexture2D(target, attachment, textarget, texture, level);

  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(BufferData) {
  Nan::HandleScope scope;

  int target = Nan::To<int>(info[0]).FromJust();
  if(info[1]->IsObject()) {
    Local<Object> obj = Local<Object>::Cast(info[1]);
    GLenum usage = Nan::To<int>(info[2]).FromJust();
    
    CHECK_ARRAY_BUFFER(obj);
    
         
            
    int element_size = 1;
    Local<ArrayBufferView> arr = Local<ArrayBufferView>::Cast(obj);
    int size = arr->ByteLength() * element_size;
    void* data = (uint8_t*)arr->Buffer()->GetContents().Data() + arr->ByteOffset();
    
    glBufferData(target, size, data, usage);
  }
  else if(info[1]->IsNumber()) {
    GLsizeiptr size = Nan::To<uint32_t>(info[1]).FromJust();
    GLenum usage = Nan::To<int>(info[2]).FromJust();
    glBufferData(target, size, NULL, usage);
  }
  info.GetReturnValue().Set(Nan::Undefined());
}


NAN_METHOD(BufferSubData) {
  Nan::HandleScope scope;

  int target = Nan::To<int>(info[0]).FromJust();
  int offset = Nan::To<int>(info[1]).FromJust();
  Local<Object> obj = Local<Object>::Cast(info[2]);
  int srcOffsetBytes = Nan::To<int>(info[3]).FromJust();
  int lengthBytes = Nan::To<int>(info[4]).FromJust();
 // cout<<"offset:"<<offset<<endl;
 // cout<<"srcOffsetBytes:"<<srcOffsetBytes<<endl;
  //cout<<"lengthBytes:"<<lengthBytes<<endl;

  int element_size = 1;
  Local<ArrayBufferView> arr = Local<ArrayBufferView>::Cast(obj);
  int size = lengthBytes==0?(arr->ByteLength() * element_size):lengthBytes;
  //cout<<"size:"<<size<<endl;
  //cout<<"bytelength:"<<arr->ByteLength()<<endl;
  void* data = (uint8_t*)arr->Buffer()->GetContents().Data() + arr->ByteOffset() + srcOffsetBytes;

  glBufferSubData(target, offset, size, data);

  info.GetReturnValue().Set(Nan::Undefined());
}


NAN_METHOD(BlendEquation) {
  Nan::HandleScope scope;

  int mode=Nan::To<int>(info[0]).FromJust();;

  glBlendEquation(mode);

  info.GetReturnValue().Set(Nan::Undefined());
}


NAN_METHOD(BlendFunc) {
  Nan::HandleScope scope;

  int sfactor=Nan::To<int>(info[0]).FromJust();;
  int dfactor=Nan::To<int>(info[1]).FromJust();;

  glBlendFunc(sfactor,dfactor);

  info.GetReturnValue().Set(Nan::Undefined());
}


NAN_METHOD(EnableVertexAttribArray) {
  Nan::HandleScope scope;

  glEnableVertexAttribArray(Nan::To<int>(info[0]).FromJust());

  info.GetReturnValue().Set(Nan::Undefined());
}


NAN_METHOD(VertexAttribPointer) {
  Nan::HandleScope scope;

  int indx = Nan::To<int>(info[0]).FromJust();
  int size = Nan::To<int>(info[1]).FromJust();
  int type = Nan::To<int>(info[2]).FromJust();
  int normalized = Nan::To<bool>(info[3]).FromJust();
  int stride = Nan::To<int>(info[4]).FromJust();
  long offset = Nan::To<int>(info[5]).FromJust();

  //    printf("VertexAttribPointer %d %d %d %d %d %d\n", indx, size, type, normalized, stride, offset);
  glVertexAttribPointer(indx, size, type, normalized, stride, (const GLvoid *)offset);

  info.GetReturnValue().Set(Nan::Undefined());
}


NAN_METHOD(ActiveTexture) {
  Nan::HandleScope scope;

  glActiveTexture(Nan::To<int>(info[0]).FromJust());
  info.GetReturnValue().Set(Nan::Undefined());
}


NAN_METHOD(DrawElements) {
  Nan::HandleScope scope;

  int mode = Nan::To<int>(info[0]).FromJust();
  int count = Nan::To<int>(info[1]).FromJust();
  int type = Nan::To<int>(info[2]).FromJust();
  GLvoid *offset = reinterpret_cast<GLvoid*>(Nan::To<uint32_t>(info[3]).FromJust());
  glDrawElements(mode, count, type, offset);
  info.GetReturnValue().Set(Nan::Undefined());
}


NAN_METHOD(Flush) {
  Nan::HandleScope scope;
  glFlush();
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(Finish) {
  Nan::HandleScope scope;
  glFinish();
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(VertexAttrib1f) {
  Nan::HandleScope scope;

  GLuint indx = Nan::To<int>(info[0]).FromJust();
  float x = (float) Nan::To<double>(info[1]).FromJust();

  glVertexAttrib1f(indx, x);
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(VertexAttrib2f) {
  Nan::HandleScope scope;

  GLuint indx = Nan::To<int>(info[0]).FromJust();
  float x = (float) Nan::To<double>(info[1]).FromJust();
  float y = (float) Nan::To<double>(info[2]).FromJust();

  glVertexAttrib2f(indx, x, y);
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(VertexAttrib3f) {
  Nan::HandleScope scope;

  GLuint indx = Nan::To<int>(info[0]).FromJust();
  float x = (float) Nan::To<double>(info[1]).FromJust();
  float y = (float) Nan::To<double>(info[2]).FromJust();
  float z = (float) Nan::To<double>(info[3]).FromJust();

  glVertexAttrib3f(indx, x, y, z);
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(VertexAttrib4f) {
  Nan::HandleScope scope;

  GLuint indx = Nan::To<int>(info[0]).FromJust();
  float x = (float) Nan::To<double>(info[1]).FromJust();
  float y = (float) Nan::To<double>(info[2]).FromJust();
  float z = (float) Nan::To<double>(info[3]).FromJust();
  float w = (float) Nan::To<double>(info[4]).FromJust();

  glVertexAttrib4f(indx, x, y, z, w);
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(VertexAttrib1fv) {
  Nan::HandleScope scope;

  int indx = Nan::To<int>(info[0]).FromJust();
  GLfloat *data = getArrayData<GLfloat>(info[1]);
  glVertexAttrib1fv(indx, data);

  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(VertexAttrib2fv) {
  Nan::HandleScope scope;

  int indx = Nan::To<int>(info[0]).FromJust();
  GLfloat *data = getArrayData<GLfloat>(info[1]);
  glVertexAttrib2fv(indx, data);

  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(VertexAttrib3fv) {
  Nan::HandleScope scope;

  int indx = Nan::To<int>(info[0]).FromJust();
  GLfloat *data = getArrayData<GLfloat>(info[1]);
  glVertexAttrib3fv(indx, data);

  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(VertexAttrib4fv) {
  Nan::HandleScope scope;

  int indx = Nan::To<int>(info[0]).FromJust();
  GLfloat *data = getArrayData<GLfloat>(info[1]);
  glVertexAttrib4fv(indx, data);

  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(BlendColor) {
  Nan::HandleScope scope;

  GLclampf r= (float) Nan::To<double>(info[0]).FromJust();
  GLclampf g= (float) Nan::To<double>(info[1]).FromJust();
  GLclampf b= (float) Nan::To<double>(info[2]).FromJust();
  GLclampf a= (float) Nan::To<double>(info[3]).FromJust();

  glBlendColor(r,g,b,a);
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(BlendEquationSeparate) {
  Nan::HandleScope scope;

  GLenum modeRGB= Nan::To<int>(info[0]).FromJust();
  GLenum modeAlpha= Nan::To<int>(info[1]).FromJust();

  glBlendEquationSeparate(modeRGB,modeAlpha);
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(BlendFuncSeparate) {
  Nan::HandleScope scope;

  GLenum srcRGB= Nan::To<int>(info[0]).FromJust();
  GLenum dstRGB= Nan::To<int>(info[1]).FromJust();
  GLenum srcAlpha= Nan::To<int>(info[2]).FromJust();
  GLenum dstAlpha= Nan::To<int>(info[3]).FromJust();

  glBlendFuncSeparate(srcRGB,dstRGB,srcAlpha,dstAlpha);
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(ClearStencil) {
  Nan::HandleScope scope;

  GLint s = Nan::To<int>(info[0]).FromJust();

  glClearStencil(s);
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(ColorMask) {
  Nan::HandleScope scope;

  GLboolean r = Nan::To<bool>(info[0]).FromJust();
  GLboolean g = Nan::To<bool>(info[1]).FromJust();
  GLboolean b = Nan::To<bool>(info[2]).FromJust();
  GLboolean a = Nan::To<bool>(info[3]).FromJust();

  glColorMask(r,g,b,a);
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(CopyTexImage2D) {
  Nan::HandleScope scope;

  GLenum target = Nan::To<int>(info[0]).FromJust();
  GLint level = Nan::To<int>(info[1]).FromJust();
  GLenum internalformat = Nan::To<int>(info[2]).FromJust();
  GLint x = Nan::To<int>(info[3]).FromJust();
  GLint y = Nan::To<int>(info[4]).FromJust();
  GLsizei width = Nan::To<int>(info[5]).FromJust();
  GLsizei height = Nan::To<int>(info[6]).FromJust();
  GLint border = Nan::To<int>(info[7]).FromJust();

  glCopyTexImage2D( target, level, internalformat, x, y, width, height, border);
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(CopyTexSubImage2D) {
  Nan::HandleScope scope;

  GLenum target = Nan::To<int>(info[0]).FromJust();
  GLint level = Nan::To<int>(info[1]).FromJust();
  GLint xoffset = Nan::To<int>(info[2]).FromJust();
  GLint yoffset = Nan::To<int>(info[3]).FromJust();
  GLint x = Nan::To<int>(info[4]).FromJust();
  GLint y = Nan::To<int>(info[5]).FromJust();
  GLsizei width = Nan::To<int>(info[6]).FromJust();
  GLsizei height = Nan::To<int>(info[7]).FromJust();

  glCopyTexSubImage2D( target, level, xoffset, yoffset, x, y, width, height);
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(CullFace) {
  Nan::HandleScope scope;

  GLenum mode = Nan::To<int>(info[0]).FromJust();

  glCullFace(mode);
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(DepthMask) {
  Nan::HandleScope scope;

  GLboolean flag = Nan::To<bool>(info[0]).FromJust();

  glDepthMask(flag);
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(DepthRange) {
  Nan::HandleScope scope;

  GLclampf zNear = (float) Nan::To<double>(info[0]).FromJust();
  GLclampf zFar = (float) Nan::To<double>(info[1]).FromJust();

  glDepthRangef(zNear, zFar);
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(DisableVertexAttribArray) {
  Nan::HandleScope scope;

  GLuint index = Nan::To<int>(info[0]).FromJust();

  glDisableVertexAttribArray(index);
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(Hint) {
  Nan::HandleScope scope;

  GLenum target = Nan::To<int>(info[0]).FromJust();
  GLenum mode = Nan::To<int>(info[1]).FromJust();

  glHint(target, mode);
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(IsEnabled) {
  Nan::HandleScope scope;

  GLenum cap = Nan::To<int>(info[0]).FromJust();

  bool ret=glIsEnabled(cap)!=0;
  info.GetReturnValue().Set(Nan::New<Boolean>(ret));
}

NAN_METHOD(LineWidth) {
  Nan::HandleScope scope;

  GLfloat width = (float) Nan::To<double>(info[0]).FromJust();

  glLineWidth(width);
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(PolygonOffset) {
  Nan::HandleScope scope;

  GLfloat factor = (float) Nan::To<double>(info[0]).FromJust();
  GLfloat units = (float) Nan::To<double>(info[1]).FromJust();

  glPolygonOffset(factor, units);
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(SampleCoverage) {
  Nan::HandleScope scope;

  GLclampf value = (float) Nan::To<double>(info[0]).FromJust();
  GLboolean invert = Nan::To<bool>(info[1]).FromJust();

  glSampleCoverage(value, invert);
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(Scissor) {
  Nan::HandleScope scope;

  GLint x = Nan::To<int>(info[0]).FromJust();
  GLint y = Nan::To<int>(info[1]).FromJust();
  GLsizei width = Nan::To<int>(info[2]).FromJust();
  GLsizei height = Nan::To<int>(info[3]).FromJust();

  glScissor(x, y, width, height);
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(StencilFunc) {
  Nan::HandleScope scope;

  GLenum func = Nan::To<int>(info[0]).FromJust();
  GLint ref = Nan::To<int>(info[1]).FromJust();
  GLuint mask = Nan::To<int>(info[2]).FromJust();

  glStencilFunc(func, ref, mask);
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(StencilFuncSeparate) {
  Nan::HandleScope scope;

  GLenum face = Nan::To<int>(info[0]).FromJust();
  GLenum func = Nan::To<int>(info[1]).FromJust();
  GLint ref = Nan::To<int>(info[2]).FromJust();
  GLuint mask = Nan::To<int>(info[3]).FromJust();

  glStencilFuncSeparate(face, func, ref, mask);
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(StencilMask) {
  Nan::HandleScope scope;

  GLuint mask = Nan::To<uint32_t>(info[0]).FromJust();

  glStencilMask(mask);
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(StencilMaskSeparate) {
  Nan::HandleScope scope;

  GLenum face = Nan::To<int>(info[0]).FromJust();
  GLuint mask = Nan::To<uint32_t>(info[1]).FromJust();

  glStencilMaskSeparate(face, mask);
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(StencilOp) {
  Nan::HandleScope scope;

  GLenum fail = Nan::To<int>(info[0]).FromJust();
  GLenum zfail = Nan::To<int>(info[1]).FromJust();
  GLenum zpass = Nan::To<int>(info[2]).FromJust();

  glStencilOp(fail, zfail, zpass);
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(StencilOpSeparate) {
  Nan::HandleScope scope;

  GLenum face = Nan::To<int>(info[0]).FromJust();
  GLenum fail = Nan::To<int>(info[1]).FromJust();
  GLenum zfail = Nan::To<int>(info[2]).FromJust();
  GLenum zpass = Nan::To<int>(info[3]).FromJust();

  glStencilOpSeparate(face, fail, zfail, zpass);
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(BindRenderbuffer) {
  Nan::HandleScope scope;

  GLenum target = Nan::To<int>(info[0]).FromJust();
  GLuint buffer = info[1]->IsNull() ? 0 : Nan::To<int>(info[1]).FromJust();

  glBindRenderbuffer(target, buffer);

  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(CreateRenderbuffer) {
  Nan::HandleScope scope;

  GLuint renderbuffers;
  glCreateRenderbuffers(1,&renderbuffers);
  #ifdef LOGGING
  cout<<"createRenderBuffer "<<renderbuffers<<endl;
  #endif
  registerGLObj(GLOBJECT_TYPE_RENDERBUFFER, renderbuffers);
  info.GetReturnValue().Set(Nan::New<Number>(renderbuffers));
}

NAN_METHOD(DeleteBuffer) {
  Nan::HandleScope scope;

  GLuint buffer = Nan::To<uint32_t>(info[0]).FromJust();

  //cout<<"deleteBuffer:"<<buffer<<endl;
  glDeleteBuffers(1,&buffer);
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(DeleteFramebuffer) {
  Nan::HandleScope scope;

  GLuint buffer = Nan::To<uint32_t>(info[0]).FromJust();

  glDeleteFramebuffers(1,&buffer);
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(DeleteProgram) {
  Nan::HandleScope scope;

  GLuint program = Nan::To<uint32_t>(info[0]).FromJust();

  glDeleteProgram(program);
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(DeleteRenderbuffer) {
  Nan::HandleScope scope;

  GLuint renderbuffer = Nan::To<uint32_t>(info[0]).FromJust();

  glDeleteRenderbuffers(1, &renderbuffer);
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(DeleteShader) {
  Nan::HandleScope scope;

  GLuint shader = Nan::To<uint32_t>(info[0]).FromJust();

  glDeleteShader(shader);
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(DeleteTexture) {
  Nan::HandleScope scope;

  GLuint texture = Nan::To<uint32_t>(info[0]).FromJust();

  glDeleteTextures(1,&texture);
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(DetachShader) {
  Nan::HandleScope scope;

  GLuint program = Nan::To<uint32_t>(info[0]).FromJust();
  GLuint shader = Nan::To<uint32_t>(info[1]).FromJust();

  glDetachShader(program, shader);
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(FramebufferRenderbuffer) {
  Nan::HandleScope scope;

  GLenum target = Nan::To<int>(info[0]).FromJust();
  GLenum attachment = Nan::To<int>(info[1]).FromJust();
  GLenum renderbuffertarget = Nan::To<int>(info[2]).FromJust();
  GLuint renderbuffer = Nan::To<uint32_t>(info[3]).FromJust();

  glFramebufferRenderbuffer(target, attachment, renderbuffertarget, renderbuffer);
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(GetVertexAttribOffset) {
  Nan::HandleScope scope;

  GLuint index = Nan::To<uint32_t>(info[0]).FromJust();
  GLenum pname = Nan::To<int>(info[1]).FromJust();
  void *ret=NULL;

  glGetVertexAttribPointerv(index, pname, &ret);
  info.GetReturnValue().Set(JS_INT(ToGLuint(ret)));
}

NAN_METHOD(IsBuffer) {
  Nan::HandleScope scope;

  info.GetReturnValue().Set(Nan::New<Boolean>(glIsBuffer(Nan::To<uint32_t>(info[0]).FromJust())!=0));
}

NAN_METHOD(IsFramebuffer) {
  Nan::HandleScope scope;

  info.GetReturnValue().Set(JS_BOOL(glIsFramebuffer(Nan::To<uint32_t>(info[0]).FromJust())!=0));
}

NAN_METHOD(IsProgram) {
  Nan::HandleScope scope;

  info.GetReturnValue().Set(JS_BOOL(glIsProgram(Nan::To<uint32_t>(info[0]).FromJust())!=0));
}

NAN_METHOD(IsRenderbuffer) {
  Nan::HandleScope scope;

  info.GetReturnValue().Set(JS_BOOL(glIsRenderbuffer( Nan::To<uint32_t>(info[0]).FromJust())!=0));
}

NAN_METHOD(IsShader) {
  Nan::HandleScope scope;

  info.GetReturnValue().Set(JS_BOOL(glIsShader(Nan::To<uint32_t>(info[0]).FromJust())!=0));
}

NAN_METHOD(IsTexture) {
  Nan::HandleScope scope;

  info.GetReturnValue().Set(JS_BOOL(glIsTexture(Nan::To<uint32_t>(info[0]).FromJust())!=0));
}

NAN_METHOD(RenderbufferStorage) {
  Nan::HandleScope scope;

  GLenum target = Nan::To<int>(info[0]).FromJust();
  GLenum internalformat = Nan::To<int>(info[1]).FromJust();
  GLsizei width = Nan::To<uint32_t>(info[2]).FromJust();
  GLsizei height = Nan::To<uint32_t>(info[3]).FromJust();

  glRenderbufferStorage(target, internalformat, width, height);
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(GetShaderSource) {
  Nan::HandleScope scope;

  int shader = Nan::To<int>(info[0]).FromJust();

  GLint len;
  glGetShaderiv(shader, GL_SHADER_SOURCE_LENGTH, &len);
  GLchar *source=new GLchar[len];
  glGetShaderSource(shader, len, NULL, source);

  Local<String> str = JS_STR(source);
  delete source;

  info.GetReturnValue().Set(str);
}

NAN_METHOD(ValidateProgram) {
  Nan::HandleScope scope;

  glValidateProgram(Nan::To<int>(info[0]).FromJust());

  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(TexSubImage2D) {
  Nan::HandleScope scope;

  GLenum target = Nan::To<int>(info[0]).FromJust();
  GLint level = Nan::To<int>(info[1]).FromJust();
  GLint xoffset = Nan::To<int>(info[2]).FromJust();
  GLint yoffset = Nan::To<int>(info[3]).FromJust();
  GLsizei width = Nan::To<int>(info[4]).FromJust();
  GLsizei height = Nan::To<int>(info[5]).FromJust();
  GLenum format = Nan::To<int>(info[6]).FromJust();
  GLenum type = Nan::To<int>(info[7]).FromJust();
  int dataSize;
  void *pixels=getImageData(info[8], dataSize);

  glTexSubImage2D(target, level, xoffset, yoffset, width, height, format, type, pixels);

  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(ReadPixels) {
  Nan::HandleScope scope;

  GLint x = Nan::To<int>(info[0]).FromJust();
  GLint y = Nan::To<int>(info[1]).FromJust();
  GLsizei width = Nan::To<int>(info[2]).FromJust();
  GLsizei height = Nan::To<int>(info[3]).FromJust();
  GLenum format = Nan::To<int>(info[4]).FromJust();
  GLenum type = Nan::To<int>(info[5]).FromJust();

  //MODIFIED BY LIAM TO SUPPORT WEBGL2 function signature
  if (!info[6]->IsNull()) {
    Local<Object> obj = Local<Object>::Cast(info[6]);
    if (!obj->IsObject()){
      GLint offset = Nan::To<int>(info[6]).FromJust();
      glReadPixels(x, y, width, height, format, type, (void*)offset);

      info.GetReturnValue().Set(Nan::Undefined());
      return;
    }
  }

  int dataSize;
  void *pixels=getImageData(info[6], dataSize);

  glReadPixels(x, y, width, height, format, type, pixels);

  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(GetTexParameter) {
  Nan::HandleScope scope;

  GLenum target = Nan::To<int>(info[0]).FromJust();
  GLenum pname = Nan::To<int>(info[1]).FromJust();

  GLint param_value=0;
  glGetTexParameteriv(target, pname, &param_value);

  info.GetReturnValue().Set(Nan::New<Number>(param_value));
}

NAN_METHOD(GetActiveAttrib) {
  Nan::HandleScope scope;

  GLuint program = Nan::To<int>(info[0]).FromJust();
  GLuint index = Nan::To<int>(info[1]).FromJust();

  char name[1024];
  GLsizei length=0;
  GLenum type;
  GLsizei size;
  glGetActiveAttrib(program, index, 1024, &length, &size, &type, name);

  Local<Array> activeInfo = Nan::New<Array>(3);
  Nan::Set(activeInfo, JS_STR("size"), JS_INT(size));
  Nan::Set(activeInfo, JS_STR("type"), JS_INT((int)type));
  Nan::Set(activeInfo, JS_STR("name"), JS_STR(name));

  info.GetReturnValue().Set(activeInfo);
}

NAN_METHOD(GetActiveUniform) {
  Nan::HandleScope scope;

  GLuint program = Nan::To<int>(info[0]).FromJust();
  GLuint index = Nan::To<int>(info[1]).FromJust();

  char name[1024];
  GLsizei length=0;
  GLenum type;
  GLsizei size;
  glGetActiveUniform(program, index, 1024, &length, &size, &type, name);

  Local<Array> activeInfo = Nan::New<Array>(3);
  Nan::Set(activeInfo, JS_STR("size"), JS_INT(size));
  Nan::Set(activeInfo, JS_STR("type"), JS_INT((int)type));
  Nan::Set(activeInfo, JS_STR("name"), JS_STR(name));

  info.GetReturnValue().Set(activeInfo);
}

NAN_METHOD(GetAttachedShaders) {
  Nan::HandleScope scope;

  GLuint program = Nan::To<int>(info[0]).FromJust();

  GLuint shaders[1024];
  GLsizei count;
  glGetAttachedShaders(program, 1024, &count, shaders);

  Local<Array> shadersArr = Nan::New<Array>(count);
  for(int i=0;i<count;i++)
    Nan::Set(shadersArr, i, JS_INT((int)shaders[i]));

  info.GetReturnValue().Set(shadersArr);
}

NAN_METHOD(GetParameter) {
  Nan::HandleScope scope;

  GLenum name = Nan::To<int>(info[0]).FromJust();

  switch(name) {
  case GL_BLEND:
  case GL_CULL_FACE:
  case GL_DEPTH_TEST:
  case GL_DEPTH_WRITEMASK:
  case GL_DITHER:
  case GL_POLYGON_OFFSET_FILL:
  case GL_SAMPLE_COVERAGE_INVERT:
  case GL_SCISSOR_TEST:
  case GL_STENCIL_TEST:
  case 0x9240 /* UNPACK_FLIP_Y_WEBGL */:
  case 0x9241 /* UNPACK_PREMULTIPLY_ALPHA_WEBGL*/:
  {
    // return a boolean
    GLboolean params;
    ::glGetBooleanv(name, &params);
    info.GetReturnValue().Set(JS_BOOL(params!=0));
    break;
  }
  case GL_DEPTH_CLEAR_VALUE:
  case GL_LINE_WIDTH:
  case GL_POLYGON_OFFSET_FACTOR:
  case GL_POLYGON_OFFSET_UNITS:
  case GL_SAMPLE_COVERAGE_VALUE:
  {
    // return a float
    GLfloat params;
    ::glGetFloatv(name, &params);
    info.GetReturnValue().Set(JS_FLOAT(params));
    break;
  }
  case GL_RENDERER:
  case GL_SHADING_LANGUAGE_VERSION:
  case GL_VENDOR:
  case GL_VERSION:
  case GL_EXTENSIONS:
  {
    // return a string
    char *params=(char*) ::glGetString(name);
    
    if(params!=NULL){
      info.GetReturnValue().Set(JS_STR(params));
    }else{
      info.GetReturnValue().Set(Nan::Undefined());
    }
    
    break;
  }
  case GL_MAX_VIEWPORT_DIMS:
  {
    // return a int32[2]
    GLint params[2];
    ::glGetIntegerv(name, params);

    Local<Array> arr=Nan::New<Array>(2);
    Nan::Set(arr, 0,JS_INT(params[0]));
    Nan::Set(arr, 1,JS_INT(params[1]));
    info.GetReturnValue().Set(arr);
    break;
  }
  case GL_SCISSOR_BOX:
  case GL_VIEWPORT:
  {
    // return a int32[4]
    GLint params[4];
    ::glGetIntegerv(name, params);

    Local<Array> arr=Nan::New<Array>(4);
    Nan::Set(arr, 0,JS_INT(params[0]));
    Nan::Set(arr, 1,JS_INT(params[1]));
    Nan::Set(arr, 2,JS_INT(params[2]));
    Nan::Set(arr, 3,JS_INT(params[3]));
    info.GetReturnValue().Set(arr);
    break;
  }
  case GL_ALIASED_LINE_WIDTH_RANGE:
  case GL_ALIASED_POINT_SIZE_RANGE:
  case GL_DEPTH_RANGE:
  {
    // return a float[2]
    GLfloat params[2];
    ::glGetFloatv(name, params);
    Local<Array> arr=Nan::New<Array>(2);
    Nan::Set(arr, 0,JS_FLOAT(params[0]));
    Nan::Set(arr, 1,JS_FLOAT(params[1]));
    info.GetReturnValue().Set(arr);
    break;
  }
  case GL_BLEND_COLOR:
  case GL_COLOR_CLEAR_VALUE:
  {
    // return a float[4]
    GLfloat params[4];
    ::glGetFloatv(name, params);
    Local<Array> arr=Nan::New<Array>(4);
    Nan::Set(arr, 0,JS_FLOAT(params[0]));
    Nan::Set(arr, 1,JS_FLOAT(params[1]));
    Nan::Set(arr, 2,JS_FLOAT(params[2]));
    Nan::Set(arr, 3,JS_FLOAT(params[3]));
    info.GetReturnValue().Set(arr);
    break;
  }
  case GL_COLOR_WRITEMASK:
  {
    // return a boolean[4]
    GLboolean params[4];
    ::glGetBooleanv(name, params);
    Local<Array> arr=Nan::New<Array>(4);
    Nan::Set(arr, 0,JS_BOOL(params[0]==1));
    Nan::Set(arr, 1,JS_BOOL(params[1]==1));
    Nan::Set(arr, 2,JS_BOOL(params[2]==1));
    Nan::Set(arr, 3,JS_BOOL(params[3]==1));
    info.GetReturnValue().Set(arr);
    break;
  }
  case GL_CURRENT_PROGRAM:
  case GL_ARRAY_BUFFER_BINDING:
  case GL_ELEMENT_ARRAY_BUFFER_BINDING:
 // case GL_DRAW_FRAMEBUFFER_BINDING:
  case GL_READ_FRAMEBUFFER_BINDING:
  case GL_FRAMEBUFFER_BINDING:
  case GL_RENDERBUFFER_BINDING:
  case GL_TEXTURE_BINDING_2D:
  case GL_TEXTURE_BINDING_CUBE_MAP:
  case GL_COPY_READ_BUFFER_BINDING:
  case GL_COPY_WRITE_BUFFER_BINDING:
  case GL_PIXEL_PACK_BUFFER_BINDING:
  case GL_PIXEL_UNPACK_BUFFER_BINDING:
  case GL_TRANSFORM_FEEDBACK_BUFFER_BINDING:
  {
    GLint params;
    ::glGetIntegerv(name, &params);
    info.GetReturnValue().Set(JS_INT(params));
    break;
  }
  default: {
    // return a long
    GLint params;
    ::glGetIntegerv(name, &params);
    info.GetReturnValue().Set(JS_INT(params));
  }
  }

  //info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(GetBufferParameter) {
  Nan::HandleScope scope;

  GLenum target = Nan::To<int>(info[0]).FromJust();
  GLenum pname = Nan::To<int>(info[1]).FromJust();

  GLint params;
  glGetBufferParameteriv(target,pname,&params);
  info.GetReturnValue().Set(JS_INT(params));
}

NAN_METHOD(GetFramebufferAttachmentParameter) {
  Nan::HandleScope scope;

  GLenum target = Nan::To<int>(info[0]).FromJust();
  GLenum attachment = Nan::To<int>(info[1]).FromJust();
  GLenum pname = Nan::To<int>(info[2]).FromJust();

  GLint params;
  glGetFramebufferAttachmentParameteriv(target,attachment, pname,&params);
  info.GetReturnValue().Set(JS_INT(params));
}

NAN_METHOD(GetProgramInfoLog) {
  Nan::HandleScope scope;

  GLuint program = Nan::To<int>(info[0]).FromJust();
  int Len = 1024;
  char Error[1024];
  glGetProgramInfoLog(program, 1024, &Len, Error);

  info.GetReturnValue().Set(JS_STR(Error));
}

NAN_METHOD(GetRenderbufferParameter) {
  Nan::HandleScope scope;

  int target = Nan::To<int>(info[0]).FromJust();
  int pname = Nan::To<int>(info[1]).FromJust();
  int value = 0;
  glGetRenderbufferParameteriv(target,pname,&value);

  info.GetReturnValue().Set(JS_INT(value));
}

NAN_METHOD(GetUniform) {
  Nan::HandleScope scope;

  GLuint program = Nan::To<int>(info[0]).FromJust();
  GLint location = Nan::To<int>(info[1]).FromJust();
  if(location < 0 ) info.GetReturnValue().Set(Nan::Undefined());

  float data[16]; // worst case scenario is 16 floats

  glGetUniformfv(program, location, data);

  Local<Array> arr=Nan::New<Array>(16);
  for(int i=0;i<16;i++)
    Nan::Set(arr, i,JS_FLOAT(data[i]));

  info.GetReturnValue().Set(arr);
}

NAN_METHOD(GetVertexAttrib) {
  Nan::HandleScope scope;

  GLuint index = Nan::To<int>(info[0]).FromJust();
  GLuint pname = Nan::To<int>(info[1]).FromJust();

  GLint value=0;

  switch (pname) {
  case GL_VERTEX_ATTRIB_ARRAY_ENABLED:
  case GL_VERTEX_ATTRIB_ARRAY_NORMALIZED:
    glGetVertexAttribiv(index,pname,&value);
    info.GetReturnValue().Set(JS_BOOL(value!=0));
    break;
  case GL_VERTEX_ATTRIB_ARRAY_SIZE:
  case GL_VERTEX_ATTRIB_ARRAY_STRIDE:
  case GL_VERTEX_ATTRIB_ARRAY_TYPE:
    glGetVertexAttribiv(index,pname,&value);
    info.GetReturnValue().Set(JS_INT(value));
    break;
  case GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING:
    glGetVertexAttribiv(index,pname,&value);
    info.GetReturnValue().Set(JS_INT(value));
    break;
  case GL_CURRENT_VERTEX_ATTRIB: {
    float vextex_attribs[4];
    glGetVertexAttribfv(index,pname,vextex_attribs);
    Local<Array> arr=Nan::New<Array>(4);
    Nan::Set(arr, 0,JS_FLOAT(vextex_attribs[0]));
    Nan::Set(arr, 1,JS_FLOAT(vextex_attribs[1]));
    Nan::Set(arr, 2,JS_FLOAT(vextex_attribs[2]));
    Nan::Set(arr, 3,JS_FLOAT(vextex_attribs[3]));
    info.GetReturnValue().Set(arr);
    break;
  }
  default:
    Nan::ThrowError("GetVertexAttrib: Invalid Enum");
  }

  //info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(GetSupportedExtensions) {
  Nan::HandleScope scope;

  char *extensions=(char*) glGetString(GL_EXTENSIONS);

  info.GetReturnValue().Set(JS_STR(extensions));
}

// TODO GetExtension(name) return the extension name if found, should be an object...
NAN_METHOD(GetExtension) {
  Nan::HandleScope scope;

  Nan::Utf8String name(info[0]);
  char *sname=*name;
  char *extensions=(char*) glGetString(GL_EXTENSIONS);
  char *ext=strcasestr(extensions, sname);
  
  if(ext==NULL){ 
      info.GetReturnValue().Set(Nan::Undefined());
  }else{
     info.GetReturnValue().Set(JS_STR(ext, (int)::strlen(sname))); 
  }
}

NAN_METHOD(CheckFramebufferStatus) {
  Nan::HandleScope scope;

  GLenum target=Nan::To<int>(info[0]).FromJust();

  info.GetReturnValue().Set(JS_INT((int)glCheckFramebufferStatus(target)));
}


/*** START OF NEW WRAPPERS ADDED BY LIAM ***/

NAN_METHOD(GetShaderPrecisionFormat) {
  Nan::HandleScope scope;

  GLenum shaderType = Nan::To<int>(info[0]).FromJust();
  GLenum precisionType = Nan::To<int>(info[1]).FromJust();

 // info.GetReturnValue().Set(JS_INT((int)glCheckFramebufferStatus(shaderType, precisionType)));

  //int program = Nan::To<int>(info[0]).FromJust();
  //Nan::Utf8String name(info[1]);
  
 // GLint range;

  GLint range[2];


  GLint precision;

  glGetShaderPrecisionFormat(shaderType, precisionType, range, &precision);

  Local<Array> precisionFormat = Nan::New<Array>(3);
  Nan::Set(precisionFormat, JS_STR("rangeMin"), JS_INT(range[0]));
  Nan::Set(precisionFormat, JS_STR("rangeMax"), JS_INT(range[1]));
  Nan::Set(precisionFormat, JS_STR("precision"), JS_INT(precision));

  info.GetReturnValue().Set(precisionFormat);
}

NAN_METHOD(TexStorage2D) {
  Nan::HandleScope scope;

  GLenum target = Nan::To<int>(info[0]).FromJust();
  GLsizei levels = Nan::To<int>(info[1]).FromJust();
  GLenum internalformat = Nan::To<int>(info[2]).FromJust();
  GLsizei width = Nan::To<int>(info[3]).FromJust();
  GLsizei height = Nan::To<int>(info[4]).FromJust();

  glTexStorage2D(target, levels, internalformat, width, height);

  info.GetReturnValue().Set(Nan::Undefined());
}

//void gl.getBufferSubData(target, srcByteOffset, ArrayBufferView dstData, optional dstOffset, optional length);
NAN_METHOD(GetBufferSubData) {
  Nan::HandleScope scope;

  GLenum target = Nan::To<int>(info[0]).FromJust();
  GLint srcByteOffset = Nan::To<int>(info[1]).FromJust();
  
  GLsizei dataSizeBytes = -1;
  void* data = getImageData(info[2], dataSizeBytes);
  
  GLsizei dstOffset = Nan::To<int>(info[3]).FromJust();
  GLsizei length = Nan::To<int>(info[4]).FromJust();

  
  GLsizei remainingBytes = dataSizeBytes - dstOffset;
  if(length != 0){
    remainingBytes = length;
  }
  if(dstOffset != 0){
    data = (void*)(((char*)data)+dstOffset);
  }
  glGetBufferSubData(target, srcByteOffset, remainingBytes, data);
}
NAN_METHOD(DeleteTransformFeedback) {
  Nan::HandleScope scope;

  GLuint tf = Nan::To<uint32_t>(info[0]).FromJust();

  glDeleteTransformFeedbacks(1,&tf);
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(CreateSampler) {
  Nan::HandleScope scope;

  GLuint sampler;
  glCreateSamplers(1, &sampler);
  #ifdef LOGGING
  cout<<"createSampler "<<sampler<<endl;
  #endif
  registerGLObj(GLOBJECT_TYPE_SAMPLER, sampler);
  info.GetReturnValue().Set(Nan::New<Number>(sampler));
}
NAN_METHOD(DeleteSampler) {
  Nan::HandleScope scope;

  GLuint sampler = Nan::To<int>(info[0]).FromJust();
  
  glDeleteSamplers(1,&sampler);

  info.GetReturnValue().Set(Nan::Undefined());
}
NAN_METHOD(SamplerParameteri) {
  Nan::HandleScope scope;

  int target = Nan::To<int>(info[0]).FromJust();
  int pname = Nan::To<int>(info[1]).FromJust();
  int param = Nan::To<int>(info[2]).FromJust();

  //cout<<"SamplerParameteri"<<target<<","<<pname<<","<<param<<endl;
  glSamplerParameteri(target, pname, param);

  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(BlitFramebuffer) {
  Nan::HandleScope scope;

  int srcX0 = Nan::To<int>(info[0]).FromJust();
  int srcY0 = Nan::To<int>(info[1]).FromJust();
  int srcX1 = Nan::To<int>(info[2]).FromJust();
  int srcY1 = Nan::To<int>(info[3]).FromJust();
  int dstX0 = Nan::To<int>(info[4]).FromJust();
  int dstY0 = Nan::To<int>(info[5]).FromJust();
  int dstX1 = Nan::To<int>(info[6]).FromJust();
  int dstY1 = Nan::To<int>(info[7]).FromJust();
  int mask = Nan::To<int>(info[8]).FromJust();
  int filter = Nan::To<int>(info[9]).FromJust();


  glBlitFramebuffer(srcX0, srcY0, srcX1, srcY1, dstX0, dstY0, dstX1, dstY1, mask, filter);

  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(BindSampler) {
  Nan::HandleScope scope;

  int unit = Nan::To<int>(info[0]).FromJust();
  int sampler = Nan::To<int>(info[1]).FromJust();

  glBindSampler(unit, sampler);

  info.GetReturnValue().Set(Nan::Undefined());
}

/*
  Nan::Utf8String code(info[1]);

  const char* codes[1];
  codes[0] = *code;
const GLchar* shaderSrc[] 
*/
NAN_METHOD(TransformFeedbackVaryings) {
  Nan::HandleScope scope;

  int program = Nan::To<int>(info[0]).FromJust();
  Local<Array> names = Local<Array>::Cast(info[1]);
  char namesArray[names->Length()][1024];
  const GLchar* namePointers[names->Length()];
  //std::vector<Nan::Utf8String> temps;
  //Nan::Utf8String* temps[names->Length()];//(Local<Value>::Cast(names->Get(i)));
  for(uint i=0;i<names->Length();++i){
    Nan::Utf8String temp(Local<Value>::Cast(Nan::Get(names, i).ToLocalChecked()));
    strcpy(namesArray[i], *temp);
    namePointers[i] = namesArray[i];
    //temps.push_back(Nan::Utf8String(Local<Value>::Cast(names->Get(i))));
  }
 
 /* for(int i=0;i<names->Length();++i){
    cout << "TransformFeedbackVaryings name: "<<namesArray[i]<<" "<<strlen(namesArray[i])<<endl;
  }*/

  int bufferMode = Nan::To<int>(info[2]).FromJust();
  //cout<<"TransformFeedbackVaryings "<<program<<" "<<bufferMode<<endl;

  glTransformFeedbackVaryings(program, names->Length(), namePointers, bufferMode);

  //delete[] temps;
 // cout<<"HERE";

  info.GetReturnValue().Set(Nan::Undefined());
}
NAN_METHOD(GetTransformFeedbackVarying) {
  Nan::HandleScope scope;

  int program = Nan::To<int>(info[0]).FromJust();
  int index = Nan::To<int>(info[1]).FromJust();

  int bufSize = 1024;
  char name[bufSize];
  GLsizei length=0;
  GLenum type;
  GLsizei size;
  
  glGetTransformFeedbackVarying(program, index, bufSize, &length, &size, &type, name);
  //info.GetReturnValue().Set(Nan::New<Number>(tf));
  info.GetReturnValue().Set(JS_STR(name, length)); 
}
NAN_METHOD(CreateTransformFeedback) {
  Nan::HandleScope scope;

  GLuint tf;
  glCreateTransformFeedbacks(1, &tf);
  #ifdef LOGGING
  cout<<"createTransformFeedback "<<tf<<endl;
  #endif
  registerGLObj(GLOBJECT_TYPE_TRANSFORM_FEEDBACK, tf);
  info.GetReturnValue().Set(Nan::New<Number>(tf));
}
NAN_METHOD(BindTransformFeedback) {
  Nan::HandleScope scope;

  int target = Nan::To<int>(info[0]).FromJust();
  int tf = Nan::To<int>(info[1]).FromJust();

  glBindTransformFeedback(target, tf);

  info.GetReturnValue().Set(Nan::Undefined());
}
NAN_METHOD(BindBufferBase) {
  Nan::HandleScope scope;
  int target = Nan::To<int>(info[0]).FromJust();
  int index = Nan::To<int>(info[1]).FromJust();
  int buffer = Nan::To<int>(info[2]).FromJust();

  glBindBufferBase(target, index, buffer);

  info.GetReturnValue().Set(Nan::Undefined());
}
NAN_METHOD(BindBufferRange) {
  Nan::HandleScope scope;

  int target = Nan::To<int>(info[0]).FromJust();
  int index = Nan::To<int>(info[1]).FromJust();
  int buffer = Nan::To<int>(info[2]).FromJust();
  int offset = Nan::To<int>(info[3]).FromJust();
  int size = Nan::To<int>(info[4]).FromJust();

  glBindBufferRange(target, index, buffer, offset, size);

  info.GetReturnValue().Set(Nan::Undefined());  
}
NAN_METHOD(BeginTransformFeedback) {
  Nan::HandleScope scope;

  GLenum primitiveMode = Nan::To<int>(info[0]).FromJust();

  glBeginTransformFeedback(primitiveMode);

  info.GetReturnValue().Set(Nan::Undefined());   
}
NAN_METHOD(EndTransformFeedback) {
  Nan::HandleScope scope;

  glEndTransformFeedback();

  info.GetReturnValue().Set(Nan::Undefined());   
}
NAN_METHOD(VertexAttribDivisor) {
  Nan::HandleScope scope;

  int index = Nan::To<int>(info[0]).FromJust();
  int divisor = Nan::To<int>(info[1]).FromJust();
  
  glVertexAttribDivisor(index, divisor);

  info.GetReturnValue().Set(Nan::Undefined());  
}
NAN_METHOD(DrawArraysInstanced) {
  Nan::HandleScope scope;

  int mode = Nan::To<int>(info[0]).FromJust();
  int first = Nan::To<int>(info[1]).FromJust();
  int count = Nan::To<int>(info[2]).FromJust();
  int instanceCount = Nan::To<int>(info[3]).FromJust();
  
  glDrawArraysInstanced(mode, first, count, instanceCount);

  info.GetReturnValue().Set(Nan::Undefined());  

}
/*NAN_METHOD(DrawElementsInstanced) {
  Nan::HandleScope scope;

  int mode = Nan::To<int>(info[0]).FromJust();
  int count = Nan::To<int>(info[1]).FromJust();
  int type = Nan::To<int>(info[2]).FromJust();
  int first = Nan::To<int>(info[3]).FromJust();
  int instanceCount = Nan::To<int>(info[4]).FromJust();
  
  glDrawElementsInstanced(mode, count, type, first, instanceCount);

  info.GetReturnValue().Set(Nan::Undefined());  

}*/
NAN_METHOD(FenceSync) {
   Nan::HandleScope scope;

  int condition = Nan::To<int>(info[0]).FromJust();
  int flags = Nan::To<int>(info[1]).FromJust();
  
  GLsync sync = glFenceSync(condition, flags);
  int syncId = registerSync(sync);

  info.GetReturnValue().Set(Nan::New<Number>(syncId));
}
NAN_METHOD(DeleteSync) {
   Nan::HandleScope scope;

  int syncId = Nan::To<int>(info[0]).FromJust();
  
  GLsync sync = getSync(syncId);
  glDeleteSync(sync);
  unregisterSync(syncId);

  info.GetReturnValue().Set(Nan::Undefined());
}
NAN_METHOD(GetSyncParameter) {
  Nan::HandleScope scope;

  int syncId = Nan::To<int>(info[0]).FromJust();
  int pname = Nan::To<int>(info[1]).FromJust();

  GLsizei bufSize = 1;
  GLint data[bufSize];
  GLsizei length=0;

  GLsync sync = getSync(syncId);
  
  glGetSynciv(sync, pname, bufSize, &length, data);

  info.GetReturnValue().Set(Nan::New<Number>(data[0]));
}
NAN_METHOD(DrawBuffers) {
  Nan::HandleScope scope;

  //int program = Nan::To<int>(info[0]).FromJust();
  Local<Array> attachments = Local<Array>::Cast(info[0]);
  GLenum bufs[attachments->Length()];
  for(uint i=0;i<attachments->Length();++i){
    bufs[i] = Nan::To<int>(Nan::Get(attachments, i).ToLocalChecked()).FromJust();
  }
 
  glDrawBuffers(attachments->Length(), bufs);

  info.GetReturnValue().Set(Nan::Undefined());
}
NAN_METHOD(TexStorage3D) {
  Nan::HandleScope scope;

  GLenum target = Nan::To<int>(info[0]).FromJust();
  GLsizei levels = Nan::To<int>(info[1]).FromJust();
  GLenum internalformat = Nan::To<int>(info[2]).FromJust();
  GLsizei width = Nan::To<int>(info[3]).FromJust();
  GLsizei height = Nan::To<int>(info[4]).FromJust();
  GLsizei depth = Nan::To<int>(info[5]).FromJust();

  glTexStorage3D(target, levels, internalformat, width, height, depth);

  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(FramebufferTextureLayer) {
  Nan::HandleScope scope;

  GLenum target = Nan::To<int>(info[0]).FromJust();
  GLenum attachment = Nan::To<int>(info[1]).FromJust();
  int tex = Nan::To<int>(info[2]).FromJust();
  int level = Nan::To<int>(info[3]).FromJust();
  int layer = Nan::To<int>(info[4]).FromJust();

  glFramebufferTextureLayer(target, attachment, tex, level, layer);

  info.GetReturnValue().Set(Nan::Undefined());
}
NAN_METHOD(CopyBufferSubData) {
  Nan::HandleScope scope;

  //readTarget, writeTarget, readOffset, writeOffset, size
  GLenum readTarget = Nan::To<int>(info[0]).FromJust();
  GLenum writeTarget = Nan::To<int>(info[1]).FromJust();
  int readOffset = Nan::To<int>(info[2]).FromJust();
  int writeOffset = Nan::To<int>(info[3]).FromJust();
  int size = Nan::To<int>(info[4]).FromJust();

  glCopyBufferSubData(readTarget, writeTarget, readOffset, writeOffset, size);

  info.GetReturnValue().Set(Nan::Undefined());  
}
NAN_METHOD(ClearBufferfv) {
  Nan::HandleScope scope;

  //readTarget, writeTarget, readOffset, writeOffset, size
  GLenum buffer = Nan::To<int>(info[0]).FromJust();
  GLint drawBuffer = Nan::To<int>(info[1]).FromJust();
  int srcOffset = Nan::To<int>(info[3]).FromJust();

  int num;
  GLfloat* ptr = getArrayData<GLfloat>(info[2],&num);
  if(num + srcOffset < 4){
    Nan::ThrowError("value array is too short, must be at least 4+srcOffset");
    return;
  }

 // cout<<"srcOffset:"<<srcOffset<<endl;
 // cout<<"num:"<<num<<endl;
  glClearBufferfv(buffer, drawBuffer, ptr);

  info.GetReturnValue().Set(Nan::Undefined());  

}
NAN_METHOD(ClearBufferData) {
  Nan::HandleScope scope;

  //readTarget, writeTarget, readOffset, writeOffset, size
  GLenum target = Nan::To<int>(info[0]).FromJust();
  GLenum internalFormat = Nan::To<int>(info[1]).FromJust();
  GLenum format = Nan::To<int>(info[2]).FromJust();
  GLenum type = Nan::To<int>(info[3]).FromJust();
/*
  cout<<"target:"<<target<<endl;
  cout<<"internalFormat:"<<internalFormat<<endl;
  //cout<<"offset:"<<offset<<endl;
  //cout<<"clearSize:"<<clearSize<<endl;
  cout<<"format:"<<format<<endl;
  cout<<"type:"<<type<<endl;
*/
  Local<ArrayBufferView> arr = Local<ArrayBufferView>::Cast(info[4]);
  //int size = arr->ByteLength();
  void* data = (uint8_t*)arr->Buffer()->GetContents().Data() + arr->ByteOffset();

  //cout<<"size:"<<size<<endl;
 // cout<<"is null"<<info[4]->IsNull()<<endl;

  glClearBufferData(target, internalFormat, format, type, data);

  info.GetReturnValue().Set(Nan::Undefined());  

}
NAN_METHOD(ClearNamedBufferData) {
  Nan::HandleScope scope;

  //readTarget, writeTarget, readOffset, writeOffset, size
  int buf = Nan::To<int>(info[0]).FromJust();
  GLenum internalFormat = Nan::To<int>(info[1]).FromJust();
  GLenum format = Nan::To<int>(info[2]).FromJust();
  GLenum type = Nan::To<int>(info[3]).FromJust();

  //cout<<"target:"<<target<<endl;
 // cout<<"buf:"<<buf<<endl;
 // cout<<"internalFormat:"<<internalFormat<<endl;
  //cout<<"offset:"<<offset<<endl;
  //cout<<"clearSize:"<<clearSize<<endl;
 // cout<<"format:"<<format<<endl;
 // cout<<"type:"<<type<<endl;

  if(info[4]->IsNull()){
   // cout<<"is null"<<endl;
    glClearNamedBufferData(buf, internalFormat, format, type, NULL);
  }else{
    Local<ArrayBufferView> arr = Local<ArrayBufferView>::Cast(info[4]);
    //int size = arr->ByteLength();
    void* data = (uint8_t*)arr->Buffer()->GetContents().Data() + arr->ByteOffset();

   // cout<<"size:"<<size<<endl;
    //cout<<"is null"<<info[4]->IsNull()<<endl;

    glClearNamedBufferData(buf, internalFormat, format, type, data);
  }

  info.GetReturnValue().Set(Nan::Undefined());  

}
NAN_METHOD(ClearBufferSubData) {
  Nan::HandleScope scope;

  //readTarget, writeTarget, readOffset, writeOffset, size
  GLenum target = Nan::To<int>(info[0]).FromJust();
  GLenum internalFormat = Nan::To<int>(info[1]).FromJust();
  GLint offset = Nan::To<int>(info[2]).FromJust();
  GLint clearSize = Nan::To<int>(info[3]).FromJust();
  GLenum format = Nan::To<int>(info[4]).FromJust();
  GLenum type = Nan::To<int>(info[5]).FromJust();

  cout<<"target:"<<target<<endl;
  cout<<"internalFormat:"<<internalFormat<<endl;
  cout<<"offset:"<<offset<<endl;
  cout<<"clearSize:"<<clearSize<<endl;
  cout<<"format:"<<format<<endl;
  cout<<"type:"<<type<<endl;

  //int num;
  //void* ptr = getArrayData<BYTE>(info[6],&num);
  int element_size = 1;
  Local<ArrayBufferView> arr = Local<ArrayBufferView>::Cast(info[6]);
  int size = arr->ByteLength() * element_size;
  void* data = (uint8_t*)arr->Buffer()->GetContents().Data() + arr->ByteOffset();

  cout<<"size:"<<size<<endl;

  glClearBufferSubData(target, internalFormat, offset, clearSize, format, type, data);

  info.GetReturnValue().Set(Nan::Undefined());  

}
NAN_METHOD(ReadBuffer) {
  Nan::HandleScope scope;
  GLenum src = Nan::To<int>(info[0]).FromJust();

  glReadBuffer(src);
  
  info.GetReturnValue().Set(Nan::Undefined());    
}
NAN_METHOD(VertexAttribIPointer) {
  Nan::HandleScope scope;

  int indx = Nan::To<int>(info[0]).FromJust();
  int size = Nan::To<int>(info[1]).FromJust();
  int type = Nan::To<int>(info[2]).FromJust();
  int stride = Nan::To<int>(info[3]).FromJust();
  long offset = Nan::To<int>(info[4]).FromJust();

  //    printf("VertexAttribPointer %d %d %d %d %d %d\n", indx, size, type, normalized, stride, offset);
  glVertexAttribIPointer(indx, size, type, stride, (const GLvoid *)offset);

  info.GetReturnValue().Set(Nan::Undefined());
}

//START OF OpenGL 4.6 functions
NAN_METHOD(BindImageTexture) {
  Nan::HandleScope scope;
  GLuint unit = Nan::To<int>(info[0]).FromJust();
  GLuint texture = Nan::To<int>(info[1]).FromJust();
  GLint level = Nan::To<int>(info[2]).FromJust();
  GLboolean layered = Nan::To<bool>(info[3]).FromJust();
  GLint layer = Nan::To<int>(info[4]).FromJust();
  GLenum access = Nan::To<int>(info[5]).FromJust();
  GLenum format = Nan::To<int>(info[6]).FromJust();

  glBindImageTexture(unit, texture, level, layered, layer, access, format);

  info.GetReturnValue().Set(Nan::Undefined());  
}
NAN_METHOD(DispatchCompute) {
  Nan::HandleScope scope;
  GLuint sx = Nan::To<int>(info[0]).FromJust();
  GLuint sy = Nan::To<int>(info[1]).FromJust();
  GLuint sz = Nan::To<int>(info[2]).FromJust();

  glDispatchCompute(sx, sy, sz);

  info.GetReturnValue().Set(Nan::Undefined());  

}
NAN_METHOD(DispatchComputeGroupSize) {
  Nan::HandleScope scope;
  GLuint sx = Nan::To<int>(info[0]).FromJust();
  GLuint sy = Nan::To<int>(info[1]).FromJust();
  GLuint sz = Nan::To<int>(info[2]).FromJust();

  GLuint wx = Nan::To<int>(info[3]).FromJust();
  GLuint wy = Nan::To<int>(info[4]).FromJust();
  GLuint wz = Nan::To<int>(info[5]).FromJust();

  glDispatchComputeGroupSizeARB(sx, sy, sz, wx, wy, wz);

  info.GetReturnValue().Set(Nan::Undefined());  

}
NAN_METHOD(MemoryBarrier) {
  Nan::HandleScope scope;
  GLuint bits = Nan::To<int>(info[0]).FromJust();

  glMemoryBarrier(bits);

  info.GetReturnValue().Set(Nan::Undefined());  

}
NAN_METHOD(ClearTexImage){
  Nan::HandleScope scope;
  GLuint tex = Nan::To<int>(info[0]).FromJust();
  GLuint level = Nan::To<int>(info[1]).FromJust();
  GLenum format = Nan::To<int>(info[2]).FromJust();
  GLenum type = Nan::To<int>(info[3]).FromJust();
  //GLenum data = Nan::To<int>(info[0]).FromJust();
  int dataSize;
  void* data = getImageData(info[4], dataSize);

  //glMemoryBarrier(bits);
  glClearTexImage(tex, level, format, type, data);

  info.GetReturnValue().Set(Nan::Undefined());    
}
NAN_METHOD(CopyImageSubData){
  Nan::HandleScope scope;
  GLuint srcTex = Nan::To<int>(info[0]).FromJust();
  GLenum srcTarget = Nan::To<int>(info[1]).FromJust();
  GLuint srcLevel = Nan::To<int>(info[2]).FromJust();
  GLuint srcX = Nan::To<int>(info[3]).FromJust();
  GLuint srcY = Nan::To<int>(info[4]).FromJust();
  GLuint srcZ = Nan::To<int>(info[5]).FromJust();

  GLuint dstTex = Nan::To<int>(info[6]).FromJust();
  GLenum dstTarget = Nan::To<int>(info[7]).FromJust();
  GLuint dstLevel = Nan::To<int>(info[8]).FromJust();
  GLuint dstX = Nan::To<int>(info[9]).FromJust();
  GLuint dstY = Nan::To<int>(info[10]).FromJust();
  GLuint dstZ = Nan::To<int>(info[11]).FromJust();

  GLuint sizeX = Nan::To<int>(info[12]).FromJust();
  GLuint sizeY = Nan::To<int>(info[13]).FromJust();
  GLuint sizeZ = Nan::To<int>(info[14]).FromJust();

  
  glCopyImageSubData(srcTex, srcTarget, srcLevel, srcX, srcY, srcZ, dstTex, dstTarget, dstLevel, dstX, dstY, dstZ, sizeX, sizeY, sizeZ);

  info.GetReturnValue().Set(Nan::Undefined());    
}
NAN_METHOD(GetTextureImage){
  Nan::HandleScope scope;
 
  GLuint tex = Nan::To<int>(info[0]).FromJust();
  GLint level = Nan::To<int>(info[1]).FromJust();
  GLenum format = Nan::To<int>(info[2]).FromJust();
  GLenum type = Nan::To<int>(info[3]).FromJust();
  GLint bufSize = Nan::To<int>(info[4]).FromJust();

  if (!info[5]->IsNull()) {
    Local<Object> obj = Local<Object>::Cast(info[5]);
    if (!obj->IsObject()){
      GLint offset = Nan::To<int>(info[5]).FromJust();
      glGetTextureImage(tex, level, format, type, bufSize, (void*) offset);
      //glReadPixels(x, y, width, height, format, type, (void*)offset);

      info.GetReturnValue().Set(Nan::Undefined());
      return;
    }
  }

  int dataSize;
  void *pixels=getImageData(info[5], dataSize);
  
  glGetTextureImage(tex, level, format, type, bufSize, pixels);

  info.GetReturnValue().Set(Nan::Undefined());    
}
NAN_METHOD(BufferStorage){
  Nan::HandleScope scope;

  GLenum target = Nan::To<int>(info[0]).FromJust();
  GLint byteSize = Nan::To<int>(info[1]).FromJust();
  GLbitfield flags = Nan::To<int>(info[3]).FromJust();

  /*if (info[2]->IsNull()) {
    glBufferStorage(target, byteSize, NULL, flags);
    cout << "null buffer storage\n";
  }else{*/

  if(info[2]->IsObject()) {

    //cout << "null buffer storage? " << info[2]->IsNull() << "\n";

    Local<Object> obj = Local<Object>::Cast(info[2]);
    Local<ArrayBufferView> arr = Local<ArrayBufferView>::Cast(obj);
    int size = byteSize==0?(arr->ByteLength()):byteSize;
    void* data = (uint8_t*)arr->Buffer()->GetContents().Data() + arr->ByteOffset();

    glBufferStorage(target, size, data, flags);
    //cout << "data buffer storage\n";
  }else{
    glBufferStorage(target, byteSize, NULL, flags);
    //cout << "null buffer storage\n";
  } 
  info.GetReturnValue().Set(Nan::Undefined());    
  
}

NAN_METHOD(NamedBufferStorage){
  Nan::HandleScope scope;

  int buf = Nan::To<int>(info[0]).FromJust();
  GLint byteSize = Nan::To<int>(info[1]).FromJust();
  GLbitfield flags = Nan::To<int>(info[3]).FromJust();

  /*if (info[2]->IsNull()) {
    glBufferStorage(target, byteSize, NULL, flags);
    cout << "null buffer storage\n";
  }else{*/

  if(info[2]->IsObject()) {

    //cout << "null buffer storage? " << info[2]->IsNull() << "\n";

    Local<Object> obj = Local<Object>::Cast(info[2]);
    Local<ArrayBufferView> arr = Local<ArrayBufferView>::Cast(obj);
    int size = byteSize==0?(arr->ByteLength()):byteSize;
    void* data = (uint8_t*)arr->Buffer()->GetContents().Data() + arr->ByteOffset();

    glNamedBufferStorage(buf, size, data, flags);
    //cout << "data buffer storage\n";
  }else{
    glNamedBufferStorage(buf, byteSize, NULL, flags);
    //cout << "null buffer storage "<<byteSize<<" "<<flags<<endl;
  } 
  info.GetReturnValue().Set(Nan::Undefined());    
  
}

NAN_METHOD(GetNamedBufferSubData) {
  Nan::HandleScope scope;

  int buf = Nan::To<int>(info[0]).FromJust();
  GLint srcByteOffset = Nan::To<int>(info[1]).FromJust();
  
  GLsizei dataSizeBytes = -1;
  void* data = getImageData(info[2], dataSizeBytes);
  
  GLsizei dstOffset = Nan::To<int>(info[3]).FromJust();
  GLsizei length = Nan::To<int>(info[4]).FromJust();

  
  GLsizei remainingBytes = dataSizeBytes - dstOffset;
  if(length != 0){
    remainingBytes = length;
  }
  if(dstOffset != 0){
    data = (void*)(((char*)data)+dstOffset);
  
  }
/*
  cout<<"buf:"<<buf<<endl;
  cout<<"srcByteOffset:"<<srcByteOffset<<endl;
  cout<<"dstOffset:"<<dstOffset<<endl;
  cout<<"remainingBytes:"<<remainingBytes<<endl;
  cout<<"dataSizeBytes:"<<dataSizeBytes<<endl;
*/
  glGetNamedBufferSubData(buf, srcByteOffset, remainingBytes, data);

  info.GetReturnValue().Set(Nan::Undefined());    
}


NAN_METHOD(NamedBufferSubData) {
  Nan::HandleScope scope;

  int buf = Nan::To<int>(info[0]).FromJust();
  int offset = Nan::To<int>(info[1]).FromJust();
  Local<Object> obj = Local<Object>::Cast(info[2]);
  int srcOffsetBytes = Nan::To<int>(info[3]).FromJust();
  int lengthBytes = Nan::To<int>(info[4]).FromJust();
 // cout<<"offset:"<<offset<<endl;
 // cout<<"srcOffsetBytes:"<<srcOffsetBytes<<endl;
  //cout<<"lengthBytes:"<<lengthBytes<<endl;

  Local<ArrayBufferView> arr = Local<ArrayBufferView>::Cast(obj);
  int size = lengthBytes==0?arr->ByteLength():lengthBytes;
  //cout<<"size:"<<size<<endl;
  //cout<<"bytelength:"<<arr->ByteLength()<<endl;
  void* data = (uint8_t*)arr->Buffer()->GetContents().Data() + arr->ByteOffset() + srcOffsetBytes;

  glNamedBufferSubData(buf, offset, size, data);

  info.GetReturnValue().Set(Nan::Undefined());
}
NAN_METHOD(TextureStorage2D) {
  Nan::HandleScope scope;

  int tex = Nan::To<int>(info[0]).FromJust();
  int levels = Nan::To<int>(info[1]).FromJust();
  int internalFormat = Nan::To<int>(info[2]).FromJust();
  int width = Nan::To<int>(info[3]).FromJust();
  int height = Nan::To<int>(info[4]).FromJust();
  

  glTextureStorage2D(tex, levels, internalFormat, width, height);

  info.GetReturnValue().Set(Nan::Undefined());
}
NAN_METHOD(TextureStorage3D) {
  Nan::HandleScope scope;

  int tex = Nan::To<int>(info[0]).FromJust();
  int levels = Nan::To<int>(info[1]).FromJust();
  int internalFormat = Nan::To<int>(info[2]).FromJust();
  int width = Nan::To<int>(info[3]).FromJust();
  int height = Nan::To<int>(info[4]).FromJust();
  int depth = Nan::To<int>(info[5]).FromJust();
  

  glTextureStorage3D(tex, levels, internalFormat, width, height, depth);

  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(TextureSubImage2D) {
  Nan::HandleScope scope;

  int tex = Nan::To<int>(info[0]).FromJust();
  GLint level = Nan::To<int>(info[1]).FromJust();
  GLint xoffset = Nan::To<int>(info[2]).FromJust();
  GLint yoffset = Nan::To<int>(info[3]).FromJust();
  GLsizei width = Nan::To<int>(info[4]).FromJust();
  GLsizei height = Nan::To<int>(info[5]).FromJust();
  GLenum format = Nan::To<int>(info[6]).FromJust();
  GLenum type = Nan::To<int>(info[7]).FromJust();
  int dataSize;
  void *pixels=getImageData(info[8], dataSize);

  glTextureSubImage2D(tex, level, xoffset, yoffset, width, height, format, type, pixels);

  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(TextureParameteri) {
  Nan::HandleScope scope;

  int tex = Nan::To<int>(info[0]).FromJust();
  int pname = Nan::To<int>(info[1]).FromJust();
  int param = Nan::To<int>(info[2]).FromJust();

  glTextureParameteri(tex, pname, param);

  info.GetReturnValue().Set(Nan::Undefined());
}
NAN_METHOD(TextureParameterf) {
  Nan::HandleScope scope;

  int tex = Nan::To<int>(info[0]).FromJust();
  int pname = Nan::To<int>(info[1]).FromJust();
  float param = Nan::To<double>(info[2]).FromJust();

  glTextureParameterf(tex, pname, param);

  info.GetReturnValue().Set(Nan::Undefined());
}


/*** END OF NEW WRAPPERS ADDED BY LIAM ***/

struct GLObj {
  GLObjectType type;
  GLuint obj;
  GLObj(GLObjectType type, GLuint obj) {
    this->type=type;
    this->obj=obj;
  }
};


map<int, GLsync> syncs;
int syncIdCounter = 0;

int registerSync(GLsync sync) {
  int syncId = syncIdCounter;
  ++syncIdCounter;
  syncs[syncId] = sync;
  return syncId;
}
void unregisterSync(int syncId) {
  syncs.erase(syncId);
}
GLsync getSync(int syncId){
  return syncs[syncId];
}

vector<GLObj*> globjs;
static bool atExit=false;

void registerGLObj(GLObjectType type, GLuint obj) {
  globjs.push_back(new GLObj(type,obj));
}


void unregisterGLObj(GLuint obj) {
  if(atExit) return;

  vector<GLObj*>::iterator it = globjs.begin();
  while(globjs.size() && it != globjs.end()) {
    GLObj *globj=*it;
    if(globj->obj==obj) {
      delete globj;
      globjs.erase(it);
      break;
    }
    ++it;
  }
}

void AtExit() {
  atExit=true;
  //glFinish();

  vector<GLObj*>::iterator it;

  #ifdef LOGGING
  cout<<"WebGL AtExit() called"<<endl;
  cout<<"  # objects allocated: "<<globjs.size()<<endl;
  it = globjs.begin();
  while(globjs.size() && it != globjs.end()) {
    GLObj *obj=*it;
    cout<<"[";
    switch(obj->type) {
    case GLOBJECT_TYPE_BUFFER: cout<<"buffer"; break;
    case GLOBJECT_TYPE_FRAMEBUFFER: cout<<"framebuffer"; break;
    case GLOBJECT_TYPE_PROGRAM: cout<<"program"; break;
    case GLOBJECT_TYPE_RENDERBUFFER: cout<<"renderbuffer"; break;
    case GLOBJECT_TYPE_SHADER: cout<<"shader"; break;
    case GLOBJECT_TYPE_TEXTURE: cout<<"texture"; break;
    };
    cout<<": "<<obj->obj<<"] ";
    ++it;
  }
  cout<<endl;
  #endif

  it = globjs.begin();
  while(globjs.size() && it != globjs.end()) {
    GLObj *globj=*it;
    GLuint obj=globj->obj;

    switch(globj->type) {
    case GLOBJECT_TYPE_PROGRAM:
      #ifdef LOGGING
      cout<<"  Destroying GL program "<<obj<<endl;
      #endif
      glDeleteProgram(obj);
      break;
    case GLOBJECT_TYPE_BUFFER:
      #ifdef LOGGING
      cout<<"  Destroying GL buffer "<<obj<<endl;
      #endif
      glDeleteBuffers(1,&obj);
      break;
    case GLOBJECT_TYPE_FRAMEBUFFER:
      #ifdef LOGGING
      cout<<"  Destroying GL frame buffer "<<obj<<endl;
      #endif
      glDeleteFramebuffers(1,&obj);
      break;
    case GLOBJECT_TYPE_RENDERBUFFER:
      #ifdef LOGGING
      cout<<"  Destroying GL render buffer "<<obj<<endl;
      #endif
      glDeleteRenderbuffers(1,&obj);
      break;
    case GLOBJECT_TYPE_SHADER:
      #ifdef LOGGING
      cout<<"  Destroying GL shader "<<obj<<endl;
      #endif
      glDeleteShader(obj);
      break;
    case GLOBJECT_TYPE_TEXTURE:
      #ifdef LOGGING
      cout<<"  Destroying GL texture "<<obj<<endl;
      #endif
      glDeleteTextures(1,&obj);
      break;
    default:
      #ifdef LOGGING
      cout<<"  Unknown object "<<obj<<endl;
      #endif
      break;
    }
    delete globj;
    ++it;
  }

  globjs.clear();
}

} // end namespace webgl
