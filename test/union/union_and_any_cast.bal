// RUN: JAVA_HOME=%java_path %testRunScript %s %nballerinacc | filecheck %s

public function print_str(string val) = external;

public function printu32(int val) = external;

public function main() {
    int | string var1 = 42;
    any result1 = <any>var1;
    print_str("RESULT=");
    printu32(<int>result1); 
}
// CHECK: RESULT=42
