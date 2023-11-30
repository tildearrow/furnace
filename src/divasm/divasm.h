struct DivASMResult {
  int line, err;
  DivASMResult():
    line(-1),
    err(0) {}
};

struct DivASMFile {
  String name;
  SafeReader* data;
};

enum DivASMTarget {
  DIV_ASM_TARGET_DUMMY=0,
  DIV_ASM_TARGET_6502,
  DIV_ASM_TARGET_SPC700
};

class DivASM {
  std::vector<DivASMFile> files;
  SafeWriter* result;

  public:
    DivASMResult getError();
    SafeWriter* assemble(String name);
    void addFile(String name, SafeReader* data);

    DivASM(DivASMTarget target);
    ~DivASM();
};
