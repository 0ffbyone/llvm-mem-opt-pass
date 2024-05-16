; ModuleID = 'no-attr-safepoint.ll'

define void @foo() {
entry:
  %0 = call i64 @llvm.read_register.i64(metadata !5)
  %1 = inttoptr i64 %0 to ptr
  %2 = getelementptr inbounds i8, ptr %1, i64 208
  %addr = load ptr, ptr %2, align 8

  call void %addr() ; CRASH

  ret void
}

; Function Attrs: nounwind readonly
declare i64 @llvm.read_register.i64(metadata)

!5 = !{!"r15"}
