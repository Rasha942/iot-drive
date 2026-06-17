#include "error_handler.hpp"
#include "inputproxy.hpp"
#include "socket.hpp"
#include <argp.h>
#include <cstdlib>
#include <cstring>
#include <err.h>
#include <iostream>
#include <memory>
#include <thread>
#include "raz_nbd.hpp"
#include "message.hpp"


static void* data;


struct buse_operations NBD::m_aop =   {
        .read = NBDRead,
        .write = NBDWrite,
        .disc = NBDDisc,
        .flush = NBDFlush,
        .trim = NBDTrim,
        .size = 0
    };    
 char NBD::m_dev_file[10] = "/dev/nbd0";
std::shared_ptr<TCPSocket> NBD::m_tcp_socket(std::make_shared<TCPSocket>(TCP_PORT, "127.0.0.1",ASocket::CLINT));

char NBD::m_arr[12 * (1024* 1024)] = {0};
struct argp NBD::m_argp = {
  .options = m_options,
  .parser = parse_opt,
  .args_doc = "SIZE DEVICE",
  .doc = "BUSE virtual block device that stores its content in memory.\n"
         "`SIZE` accepts suffixes K, M, G. `DEVICE` is path to block device, for example \"/dev/nbd0\".",
};
struct argp_option NBD::m_options[] = {
  {"verbose", 'v', 0, 0, "Produce verbose output", 0},
  {0},
};

void* NBD::m_data = NULL;

int NBD::NBDRead(void *buf, u_int32_t len, u_int64_t offset, void *userdata)
{
    (void)userdata;
        printf("Debug: Reading %u bytes at offset %llu\n", len, offset);


    NBDReadMsg read_msg(offset,len);
    
    std::cout << "NBD READ MSG GET BUFF SUZE = " << read_msg.GetBufferSize() << std::endl;
    std::shared_ptr<char[]> sent_buffer(new char [read_msg.GetBufferSize()],std::default_delete<char[]>());
    char* sent_buff_ptr = sent_buffer.get();
    read_msg.ToBuffer(sent_buff_ptr);
    m_tcp_socket->Send(sent_buff_ptr, read_msg.GetBufferSize());
    m_tcp_socket->Recive(buf, len);
    return 0;
}

int NBD::NBDWrite(const void *buf, u_int32_t len, u_int64_t offset, void *userdata)
{
    (void)userdata;
    
    printf("Debug: Writring %u bytes at offset %llu\n", len, offset);
    


    NBDWriteMsg write_msg(offset,len,(char*)buf);
    std::shared_ptr<char[]> sent_buffer(new char [write_msg.GetBufferSize()],std::default_delete<char[]>());
    char* sent_buff_ptr = sent_buffer.get();
    write_msg.ToBuffer(sent_buff_ptr);
    m_tcp_socket->Send(sent_buff_ptr, write_msg.GetBufferSize());
    std::shared_ptr<char[]> recv_buff(new char[len],std::default_delete<char[]>());
    m_tcp_socket->Recive(recv_buff.get(), len);
 
        return 0;
    
    
    
}

void NBD::NBDDisc(void *userdata){}
int NBD::NBDFlush(void *userdata){return 0;}
int NBD::NBDTrim(u_int64_t from, u_int32_t len, void *userdata){return 0;}

int NBD::Run()
{
      struct arguments arguments = 
    {
        .size = 0,
        .verbose = 0
    };
    char file[55] = "iot-drive";
    char size[4] = "12M";
    char* size_and_file[3] = {file,size,m_dev_file};
   std::cout << "Args: " << size_and_file[0] << ", " << size_and_file[1] << ", " << size_and_file[2] << std::endl;

    argp_parse(&m_argp, 3, size_and_file, 0, 0, &arguments);
    m_aop.size = arguments.size;
    if (arguments.size == 0) {  // Validate size was set correctly
        fprintf(stderr, "Error: Size argument parsing failed.\n");
        return -1;
    }
    //std::cout<<__PRETTY_FUNCTION__<< "  size: " << m_aop.size << std::endl;
    data = malloc(m_aop.size);
    memset(data, 0, m_aop.size);
    arguments.device = m_dev_file;
    m_tcp_socket->TCPConnect();
     int ret = buse_main(m_dev_file, &m_aop, (void*)&arguments.verbose); 
     //std::cout << "after buse_main!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << std::endl;
     return ret;
}
bool NBD::IsBusy() 
{
        return access("/dev/nbd0", F_OK) != 0;

    // return system("nbd-client -c " "/dev/nbd0" " > /dev/null 2>&1") == 0;
}

void NBD::MKFSAndMount()
{

  std::this_thread::sleep_for(std::chrono::seconds(5));

  std::cout << "Waiting for " << "/dev/nbd0" << std::endl;

    while (NBD::IsBusy()) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    std::cout << "/dev/nbd0 is ready, running mkfs.ext2..." << std::endl;
    CheckAndLog::NotEqualToCheck(system("mkfs.ext2  /dev/nbd0"), 0,"mkfs.ext2");
    std::cout << "mkfs done, mounting nbd..." << std::endl;
    CheckAndLog::NotEqualToCheck(system("mount /dev/nbd0  /mnt"), 0,"mkfs.ext2");
   
}


unsigned long long NBD::strtoull_with_prefix(const char * str, char * * end) {
  unsigned long long v = strtoull(str, end, 0);
  switch (**end) {
    case 'K':
      v *= 1024;
      *end += 1;
      break;
    case 'M':
      v *= 1024 * 1024;
      *end += 1;
      break;
    case 'G':
      v *= 1024 * 1024 * 1024;
      *end += 1;
      break;
  }
  return v;
}


error_t NBD::parse_opt(int key, char *arg, struct argp_state *state) {
  struct arguments *arguments = (struct arguments*)state->input;
  char * endptr;

  switch (key) {

    case 'v':
      arguments->verbose = 1;
      break;

    case ARGP_KEY_ARG:
      switch (state->arg_num) {

        case 0:
          arguments->size = strtoull_with_prefix(arg, &endptr);
          if (*endptr != '\0') {
            /* failed to parse integer */
            errx(EXIT_FAILURE, "SIZE must be an integer");
          }
          break;

        case 1:
          arguments->device = arg;
          break;

        default:
          /* Too many arguments. */
          return ARGP_ERR_UNKNOWN;
      }
      break;

    case ARGP_KEY_END:
      if (state->arg_num < 2) {
        warnx("not enough arguments");
        argp_usage(state);
      }
      break;

    default:
      return ARGP_ERR_UNKNOWN;
  }
  std::cout << "Parsed size: " << arguments->size << " Parsed device: " << arguments->device << std::endl;

  return 0;
}
