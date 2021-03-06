#include "parflow/pfdata.hpp"
#include "pfutil.hpp"

#include <array>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>



#define WRITEINT(V,f) {uint32_t temp = bswap32(V); \
                         fwrite(&temp, 4, 1, f);}
#define READINT(V,f,err) {uint32_t buf; \
                         err = fread(&buf, 4, 1, f);\
                         uint32_t temp =  bswap32(buf);\
                         V = (int)temp;}
#define WRITEDOUBLE(V,f) {uint64_t t1 = *(uint64_t*)&V;\
                         t1 =  bswap64(t1); \
                         fwrite(&t1, 8, 1, f);}
#define READDOUBLE(V,f,err) {uint64_t buf; \
                         err = fread(&buf, 8, 1, f);\
                         uint64_t temp =  bswap64(buf);\
                         V = *(double*)&temp;}



PFData::PFData() {
    m_fp = nullptr;
    m_data = nullptr;
    m_X = m_Y = m_Z = 0.0;
    m_dX = m_dY = m_dZ = 1.0;
    m_p = m_q = m_r = 1;
}

PFData::PFData(std::string filename) : PFData{} {
    m_filename = filename;

}

PFData::PFData(double *data, int nz, int ny, int nx) {
    m_data = data;
    m_nx = nx;
    m_ny = ny;
    m_nz = nz;
    m_X = m_Y = m_Z = 0.0;
    m_dX = m_dY = m_dZ = 1.0;
    m_p = m_q = m_r = 1;
    m_fp = nullptr;
}

int PFData::loadHeader() {

    m_fp = fopen( m_filename.c_str(), "rb");
    if(m_fp == nullptr){
        std::string err{"Error opening file: \"" + m_filename + "\""};
        perror(err.c_str());
        return 1;
    }
    /* read in header information */
    int errcheck;
    READDOUBLE(m_X,m_fp,errcheck);
    if(!errcheck){perror("Error Reading Header"); return 1;}
    READDOUBLE(m_Y,m_fp,errcheck);
    if(!errcheck){perror("Error Reading Header"); return 1;}
    READDOUBLE(m_Z,m_fp,errcheck);
    if(!errcheck){perror("Error Reading Header"); return 1;}
    READINT(m_nx,m_fp,errcheck);
    if(!errcheck){perror("Error Reading Header"); return 1;}
    READINT(m_ny,m_fp,errcheck);
    if(!errcheck){perror("Error Reading Header"); return 1;}
    READINT(m_nz,m_fp,errcheck);
    if(!errcheck){perror("Error Reading Header"); return 1;}
    READDOUBLE(m_dX,m_fp,errcheck);
    if(!errcheck){perror("Error Reading Header"); return 1;}
    READDOUBLE(m_dY,m_fp,errcheck);
    if(!errcheck){perror("Error Reading Header"); return 1;}
    READDOUBLE(m_dZ,m_fp,errcheck);
    if(!errcheck){perror("Error Reading Header"); return 1;}
    READINT(m_numSubgrids,m_fp,errcheck);
    if(!errcheck){perror("Error Reading Header"); return 1;}

    return 0;
}

double PFData::getX() const {
    return m_X;
}

void PFData::setX(double X) {
    m_X = X;
}

double PFData::getY() const {
    return m_Y;
}

void PFData::setY(double Y) {
    m_Y = Y;
}

double PFData::getZ() const {
    return m_Z;
}

void PFData::setZ(double Z) {
    m_Z = Z;
}

int PFData::getNX() const {
    return m_nx;
}

void PFData::setNX(int Nx) {
    m_nx = Nx;
}

int PFData::getNY() const {
    return m_ny;
}

void PFData::setNY(int Ny) {
    m_ny = Ny;
}

int PFData::getNZ() const {
    return m_nz;
}

void PFData::setNZ(int Nz) {
    m_nz = Nz;
}

double PFData::getDX() const {
    return m_dX;
}

void PFData::setDX(double DX) {
    m_dX = DX;
}

double PFData::getDY() const {
    return m_dY;
}

void PFData::setDY(double DY) {
    m_dY = DY;
}

double PFData::getDZ() const {
    return m_dZ;
}

void PFData::setDZ(double DZ) {
    m_dZ = DZ;
}

int PFData::getNumSubgrids() const {
    return m_numSubgrids;
}

void PFData::setNumSubgrids(int NumSubgrids) {
    m_numSubgrids = NumSubgrids;
}

double* PFData::getSubgridData(int grid) {
    return nullptr;
}

int PFData::getCoordinateDatum(int x, int y, int z, double *value) {
    return 0;
}

double PFData::operator()(int x, int y, int z) {
    return m_data[z*m_ny*m_nx+y*m_nx+x];
}

double* PFData::getData() {
    return m_data;
}

const double* PFData::getData() const{
    return m_data;
}

int PFData::loadData() {
    int nsg;
    //subgrid variables
    int x,y,z,nx,ny,nz,rx,ry,rz;
    if(m_fp  == nullptr){
        return 1;
    }
    m_data = (double*)malloc(sizeof(double)*m_nx*m_ny*m_nz);
    if(m_data == nullptr){
        return 2;
    }
    for (nsg = 0;nsg<m_numSubgrids; nsg++){
        // read subgrid header
        int errcheck;
        READINT(x,m_fp,errcheck);
        if(!errcheck){perror("Error Reading Subgrid Header"); return 1;}
        READINT(y,m_fp,errcheck);
        if(!errcheck){perror("Error Reading Subgrid Header"); return 1;}
        READINT(z,m_fp,errcheck);
        if(!errcheck){perror("Error Reading Subgrid Header"); return 1;}
        READINT(nx,m_fp,errcheck);
        if(!errcheck){perror("Error Reading Subgrid Header"); return 1;}
        READINT(ny,m_fp,errcheck);
        if(!errcheck){perror("Error Reading Subgrid Header"); return 1;}
        READINT(nz,m_fp,errcheck);
        if(!errcheck){perror("Error Reading Subgrid Header"); return 1;}
        READINT(rx,m_fp,errcheck);
        if(!errcheck){perror("Error Reading Subgrid Header"); return 1;}
        READINT(ry,m_fp,errcheck);
        if(!errcheck){perror("Error Reading Subgrid Header"); return 1;}
        READINT(rz,m_fp,errcheck);
        if(!errcheck){perror("Error Reading Subgrid Header"); return 1;}
        if(nsg == m_numSubgrids-1){
            m_p = m_nx/nx;
            m_q = m_ny/ny;
            m_r = m_nz/nz;
        }

        // read values for subgrid
        // qq is the location of the subgrid
        int qq = z*m_nx*m_ny + y*m_nx + x;
        int k,i,j;
        //int index = qq;//+k*nx*ny+i*nx;
        for (k=0; k<nz; k++){
            for(i=0;i<ny;i++){
                // read full "pencil"
                int index = qq+k*m_nx*m_ny+i*m_nx;
                uint64_t* buf = (uint64_t*)&(m_data[index]);
                int read_count = fread(buf,8,nx,m_fp);
                if(read_count != nx){
                    perror("Error Reading Data, File Ended Unexpectedly");
                    return 1;
                }
                // handle byte order
                //uint64_t* buf = (uint64_t*)&(m_data[index]);
                for(j=0;j<nx;j++){
                    uint64_t tmp = buf[j];
                    tmp = bswap64(tmp);
                    m_data[index+j] = *(double*)(&tmp);
                }
            }
        }
    }
    return 0;
}

void PFData::close() {
  if(m_fp == nullptr){
      return;
  }
  fclose(m_fp);
  m_fp = nullptr;
  return;
}


int PFData::writeFile(const std::string filename) {
    std::vector<long> _offsets((m_p*m_q*m_r) + 1);
    return writeFile(filename, _offsets);
}

int PFData::writeFile(const std::string filename, std::vector<long> &byte_offsets) {

    std::FILE* fp = std::fopen(filename.c_str(), "wb");
    if(fp == nullptr){
        std::string err{"Error opening file: \"" + filename + "\""};
        perror(err.c_str());
        return 1;
    }

    // calculate the number of subgrids.
    m_numSubgrids = m_p * m_q * m_r;
    WRITEDOUBLE(m_X,fp);
    WRITEDOUBLE(m_Y,fp);
    WRITEDOUBLE(m_Z,fp);
    WRITEINT(m_nx,fp);
    WRITEINT(m_ny,fp);
    WRITEINT(m_nz,fp);
    WRITEDOUBLE(m_dX,fp);
    WRITEDOUBLE(m_dY,fp);
    WRITEDOUBLE(m_dZ,fp);
    WRITEINT(m_numSubgrids,fp);
    int max_x_extent =calcExtent(m_nx,m_p,0);
    std::vector<double> writeBuf(max_x_extent);
    // now write the subgrids one at a time
    // this iterates over the subgrids in order
    int nsg=0;
    byte_offsets[0] = 0;
    int sg_count = 1;
    for(int nsg_z=0;nsg_z<m_r;nsg_z++){
        for(int nsg_y=0;nsg_y<m_q;nsg_y++) {
            for (int nsg_x = 0;nsg_x<m_p;nsg_x++) {
                //Write byte offset of chunk from header
                // This subgrid starts at x,y,z
                // The number of items in the x-direction of each block is m_nx/m_p + [1|0]
                // The 1 is added for the last m_nx%m_p blocks
                int x = m_X + calcOffset(m_nx,m_p,nsg_x);
                int y = m_Y + calcOffset(m_ny,m_q,nsg_y);
                int z = m_Z + calcOffset(m_nz,m_r,nsg_z);
                // x,y,z of lower lefthand corner
                WRITEINT(x, fp);
                WRITEINT(y, fp);
                WRITEINT(z, fp);
                // nx,ny,nz extents of each direction
                int x_extent =calcExtent(m_nx,m_p,nsg_x);
                WRITEINT(x_extent, fp);
                WRITEINT(calcExtent(m_ny,m_q,nsg_y), fp);
                WRITEINT(calcExtent(m_nz,m_r,nsg_z), fp);
                // subgrid  location in 3D grid
                WRITEINT(1, fp);
                WRITEINT(1, fp);
                WRITEINT(1, fp);

                int  ix,iy,iz;
                for(iz=calcOffset(m_nz,m_r,nsg_z); iz < calcOffset(m_nz,m_r,nsg_z+1);iz++){
                    for(iy=calcOffset(m_ny,m_q,nsg_y); iy < calcOffset(m_ny,m_q,nsg_y+1);iy++){

                        uint64_t* buf = (uint64_t*)&(m_data[iz*m_nx*m_ny+iy*m_nx+calcOffset(m_nx,m_p,nsg_x)]);
                        int j;
                        for(j=0;j<x_extent;j++){
                            uint64_t tmp = buf[j];
                            tmp = bswap64(tmp);
                            writeBuf[j] = *(double*)(&tmp);
                        }
                        int written = fwrite(writeBuf.data(),sizeof(double),x_extent,fp);
                        if(written != x_extent){
                            std::fclose(fp);
                            std::cerr << "Error writing subgrid data to file " << filename << "\n";
                            perror("");
                            return 1;
                        }
                    }
                }
                byte_offsets[sg_count] = std::ftell(fp);
                sg_count++;
            }
        }
        nsg++;
    }
    fclose(fp);
    return 0;
}
int calcExtent(int extent, int block_count, int block_idx) {
    int lx = extent % block_count;
    int bx = extent / block_count;
    int block_extent = bx;
    if (block_idx < lx) {
        block_extent++;
    }
    return block_extent;
}
int calcOffset(int extent, int block_count, int block_idx) {
    int lx = extent % block_count;
    int bx = extent / block_count;
    int offset =  block_idx * bx;
    if (block_idx < lx) {
        offset += block_idx;
    }else{
        offset += lx;
    }
    return offset;
}

int PFData::writeFile() {
    return 0;
}

int PFData::distFile(int P, int Q, int R, const std::string outFile) {
    loadHeader();
    loadData();
    m_p = P;
    m_q = Q;
    m_r = R;
    // create array to hold byte offset for blocks
    std::vector<long> offsets((P*Q*R)+1);
    //create the filestream for the .dist file
    std::fstream distFile(outFile + ".dist", std::ios::trunc | std::ios::out) ;   //Clear file if it exists
    if(!distFile){
        perror("Error creating distfile");
        return 1;
    }

    //write the blocked pfb file to disk, collecting the block offsets
    int rtnVal = writeFile(outFile, offsets);

    //write the block offsets to the .dist file
    for (int i = 0; i<=P*Q*R; i++){
        distFile << offsets[i] << "\n";
    }

    return rtnVal;
}

PFData::differenceType PFData::compare(const PFData& otherObj, std::array<int, 3>* diffIndex) const{
    //Check relevant header data
    if(otherObj.getZ()  != getZ())  return differenceType::z;
    if(otherObj.getY()  != getY())  return differenceType::y;
    if(otherObj.getX()  != getX())  return differenceType::x;

    if(otherObj.getDZ() != getDZ()) return differenceType::dZ;
    if(otherObj.getDY() != getDY()) return differenceType::dY;
    if(otherObj.getDX() != getDX()) return differenceType::dX;

    if(otherObj.getNZ() != getNZ()) return differenceType::nZ;
    if(otherObj.getNY() != getNY()) return differenceType::nY;
    if(otherObj.getNX() != getNX()) return differenceType::nX;


    //Check for differences in the data array
    //@@TODO: Do we want to handle invalid data?
    assert(otherObj.getData() && getData());
    assert(getNZ() > 0 && getNY() > 0 && getNX() > 0);

    const double* dataOther = otherObj.getData();
    const double* dataSelf = getData();
    const int dataSize = getNZ() * getNY() * getNX();

    for(int i = 0; i < dataSize; ++i){
        if(dataOther[i] != dataSelf[i]){
            if(diffIndex){  //Only write if not null
                *diffIndex = unflattenIndex(i);
            }

            return differenceType::data;
        }
    }

    return differenceType::none;
}

std::array<int, 3> PFData::unflattenIndex(int index) const{
    if(index >= getNZ() * getNY() * getNX() || index < 0){  //Invalid index, @@TODO assert instead?
        return {-1, -1, -1};
    }

    const int z = index / (getNX() * getNY());
    index -= z * getNX() * getNY();

    const int y = index / getNX();
    index -= y * getNX();

    const int x = index;    //index remainder is x value

    //Sanity check
    assert(z < getNZ() && z >= 0);
    assert(y < getNY() && y >= 0);
    assert(x < getNX() && x >= 0);

    return {z, y, x};
}

std::string PFData::getFilename() const{
    return m_filename;
}

void PFData::setData(double *data) {
    m_data = data;
}

int PFData::getP() const {
    return m_p;
}

int PFData::getQ() const {
    return m_q;
}

int PFData::getR() const {
    return m_r;
}

void PFData::setP(int P) {
    m_p = P;
}

void PFData::setQ(int Q) {
    m_q = Q;
}

void PFData::setR(int R) {
    m_r = R;
}
