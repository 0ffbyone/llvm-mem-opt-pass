; ModuleID = './tests/IR/no-intrinsic.ll'
source_filename = "./tests/IR/no-intrinsic.ll"

define void @foo() {
entry:
  %0 = call i64 @llvm.read_register.i64(metadata !0)
  %1 = inttoptr i64 %0 to ptr
  %2 = getelementptr inbounds i8, ptr %1, i64 208
  %addr = load ptr, ptr %2, align 8
  call void %addr()
  ret void
}

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(read)
declare i64 @llvm.read_register.i64(metadata) #0

attributes #0 = { nocallback nofree nosync nounwind willreturn memory(read) }

!0 = !{!"r15"}
