#define main room001_unused_main
#include "../room-001/room001.cpp"
#undef main
int main(int argc,char** argv){
 if(argc!=3){std::cerr<<"usage: convergence_room input-f64 output-f64\n";return 1;}
 constexpr int fs=48000,n=fs*8;
 std::ifstream in(argv[1],std::ios::binary);std::vector<double> src(n);in.read(reinterpret_cast<char*>(src.data()),std::streamsize(n*sizeof(double)));if(!in||in.peek()!=EOF){std::cerr<<"input must be exactly 384000 little-endian float64 samples\n";return 2;}
 auto audio=render(fs,src,15,2.8,1.0);std::ofstream out(argv[2],std::ios::binary);if(!out)return 3;for(auto s:audio){out.write(reinterpret_cast<const char*>(&s[0]),sizeof(double));out.write(reinterpret_cast<const char*>(&s[1]),sizeof(double));}return out?0:4;
}
