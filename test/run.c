// Headless Mesen runner: run.c <core.so> <rom> <frames> <outprefix> [dump frames...]
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdarg.h>
#include "libretro.h"
static const void *fb; static unsigned fw,fh; static size_t fp; static int fmt=0;
static void lg(enum retro_log_level l,const char*f,...){}
static bool env(unsigned c, void *d){
  switch(c){
   case RETRO_ENVIRONMENT_SET_PIXEL_FORMAT: fmt=*(int*)d; return true;
   case RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY: case RETRO_ENVIRONMENT_GET_SAVE_DIRECTORY: *(const char**)d="/tmp"; return true;
   case RETRO_ENVIRONMENT_GET_LOG_INTERFACE: ((struct retro_log_callback*)d)->log=lg; return true;
   case RETRO_ENVIRONMENT_GET_VARIABLE: { struct retro_variable*v=d;
      if(!strcmp(v->key,"mesen_overscan_vertical")||0){v->value="None";return true;} v->value=NULL; return false;}
   case RETRO_ENVIRONMENT_GET_VARIABLE_UPDATE: *(bool*)d=false; return true;
   default: return false; }
}
static void vid(const void*d,unsigned w,unsigned h,size_t p){ if(d){fb=d;fw=w;fh=h;fp=p;} }
static void aud(int16_t l,int16_t r){}
static FILE*af; static size_t audb(const int16_t*d,size_t n){if(af)fwrite(d,4,n,af);return n;}
static void poll(void){}
static int curFrame; static int sc[16384][3],nsc;
static int16_t st(unsigned p,unsigned d,unsigned i,unsigned id){ if(p||d!=RETRO_DEVICE_JOYPAD) return 0;
  for(int k=0;k<nsc;k++) if(curFrame>=sc[k][0]&&curFrame<sc[k][1]&&(sc[k][2]>>id&1)) return 1; return 0;}
#define S(n) n##_t n; 
typedef void(*f0)(void);
int main(int c,char**v){
  void*h=dlopen(v[1],RTLD_NOW); if(!h){puts(dlerror());return 1;}
  #define L(n) dlsym(h,#n)
  ((void(*)(retro_environment_t))L(retro_set_environment))(env);
  ((f0)L(retro_init))();
  ((void(*)(retro_video_refresh_t))L(retro_set_video_refresh))(vid);
  ((void(*)(retro_audio_sample_t))L(retro_set_audio_sample))(aud);
  ((void(*)(retro_audio_sample_batch_t))L(retro_set_audio_sample_batch))(audb);
  ((void(*)(retro_input_poll_t))L(retro_set_input_poll))(poll);
  ((void(*)(retro_input_state_t))L(retro_set_input_state))(st);
  static char buf[1<<20]; FILE*f=fopen(v[2],"rb"); size_t n=fread(buf,1,sizeof buf,f); fclose(f);
  struct retro_game_info gi={v[2],buf,n,NULL};
  if(!((bool(*)(const struct retro_game_info*))L(retro_load_game))(&gi)){puts("load fail");return 1;}
  if(getenv("AUDIO")){af=fopen(getenv("AUDIO"),"wb");struct retro_system_av_info av;((void(*)(struct retro_system_av_info*))L(retro_get_system_av_info))(&av);fprintf(stderr,"rate %f fps %f\n",av.timing.sample_rate,av.timing.fps);}
  uint8_t*(*mem)(unsigned)=(void*)L(retro_get_memory_data);
  uint8_t*ram=mem(RETRO_MEMORY_SYSTEM_RAM);
  if(getenv("INPUT")){FILE*q=fopen(getenv("INPUT"),"r");while(fscanf(q,"%d %d %d",&sc[nsc][0],&sc[nsc][1],&sc[nsc][2])==3)nsc++;fclose(q);}
  uint8_t*sv=mem(RETRO_MEMORY_SAVE_RAM); size_t svn=((size_t(*)(unsigned))L(retro_get_memory_size))(RETRO_MEMORY_SAVE_RAM); fprintf(stderr,"save ram %zu\n",svn);
  if(getenv("SAVE")&&sv){FILE*q=fopen(getenv("SAVE"),"rb");if(q){fread(sv,1,svn,q);fclose(q);}}
  int frames=atoi(v[3]);
  for(int i=1;i<=frames;i++){
    curFrame=i; ((f0)L(retro_run))(); /*BLANKBOLT*/
    int dump=0; for(int k=5;k<c;k++) if(atoi(v[k])==i) dump=1;
    if(dump){ char p[300]; sprintf(p,"%s_%d.ppm",v[4],i); FILE*o=fopen(p,"wb"); fprintf(o,"P6 %u %u 255\n",fw,fh);
      for(unsigned y=0;y<fh;y++)for(unsigned x=0;x<fw;x++){
        uint32_t px=fmt==1?((uint32_t*)((char*)fb+y*fp))[x]:0; uint8_t rgb[3]={px>>16,px>>8,px}; fwrite(rgb,1,3,o);} fclose(o);}
    if(getenv("RAW")){static FILE*o; if(!o)o=fopen(getenv("RAW"),"wb"); uint8_t rowb[256]; for(unsigned y=0;y<fh;y++){for(unsigned x=0;x<fw;x++){uint32_t p=((uint32_t*)((char*)fb+y*fp))[x]; rowb[x]=(p&0xffffff)>0x202020?1:0;} fwrite(rowb,1,256,o);}
      uint8_t rec[40]; memcpy(rec,ram+0x320,12); rec[12]=ram[0x306];rec[13]=ram[0x307];rec[14]=ram[0x304];rec[15]=getenv("VIS")?ram[0x308]:ram[0x30A];rec[16]=ram[0x302];rec[17]=ram[0x300]; rec[18]=ram[0x301]; rec[19]=ram[0x31E]; rec[20]=ram[0x30C]; rec[21]=ram[0x30D]; rec[22]=ram[0x30E]; rec[23]=ram[0x30F]; memcpy(rec+24,ram+0x330,8);rec[32]=ram[0x5B4];rec[33]=ram[0x31B];rec[34]=ram[0x31C];rec[35]=ram[0x31D];rec[34]=ram[0x5AF];rec[35]=ram[0x5D3];rec[36]=ram[0x604];rec[37]=ram[0x634];rec[38]=ram[0x624];rec[39]=ram[0x654];fwrite(rec,1,40,o);fflush(o);}
    if(i%60==0||i==frames) printf("t=%d frames=%d nmi=%d tiles=%d peak=%d yaw=%d pitch=%d vis=%02x stage=%d xf=%d snd=%d,%d,%d,%d,%d,%d mode=%d flags=%d slow=%d near=%d clip=%d alive=%x score=%d shields=%d hits=%d crash=%d shots=%d bolthits=%d\n",i,ram[0x300]|ram[0x301]<<8,ram[0x302]|ram[0x303]<<8,ram[0x304],ram[0x305],ram[0x306],ram[0x307],ram[0x308],ram[0x309],ram[0xB0],ram[0x314],ram[0x315],ram[0x316],ram[0x317],ram[0x318],ram[0x319],ram[0x31E],ram[0x31B],ram[0x30A],ram[0x30B],ram[0x30C],ram[0x30D],ram[0x30E],ram[0x30F],ram[0x310],ram[0x311],ram[0x312],ram[0x313]);
  }
  if(getenv("SAVE")&&sv){FILE*q=fopen(getenv("SAVE"),"wb");fwrite(sv,1,svn,q);fclose(q);}
  return 0;
}
