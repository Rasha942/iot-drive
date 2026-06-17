#ifndef __RAZ_NBD_HPP__
#define __RAZ_NBD_HPP__

#include "buse.hpp"
#include "socket.hpp"
#include <memory>



class NBD
{
public:
  NBD();
  static int Run();
  static void MKFSAndMount();
// private:
  static int NBDRead(void *buf, u_int32_t len, u_int64_t offset, void *userdata);
  static int NBDWrite(const void *buf, u_int32_t len, u_int64_t offset, void *userdata);
  static void NBDDisc(void *userdata);
  static int NBDFlush(void *userdata);
  static int NBDTrim(u_int64_t from, u_int32_t len, void *userdata);
  static unsigned long long strtoull_with_prefix(const char * str, char * * end);
  static error_t parse_opt(int key, char *arg, struct argp_state *state); 
  static bool IsBusy();
    static  struct buse_operations m_aop; 
    static struct argp m_argp;
    struct arguments {
  unsigned long long size;
  char * device;
  int verbose;
};
  static struct argp_option m_options[];

    static char m_dev_file[10];
    static std::shared_ptr<TCPSocket> m_tcp_socket;
    static void*  m_data;
    static char m_arr[12 * (1024* 1024)];
};




#endif //__RAZ_NBD_HPP__