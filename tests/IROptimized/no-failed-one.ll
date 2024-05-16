; ModuleID = './tests/IR/no-failed-one.ll'
source_filename = "panda-llvmaot-module"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@__aot_got = protected global [0 x i64] zeroinitializer, section ".aot_got"

; Function Attrs: nounwind
define void @gc.safepoint_poll() #0 {
bb:
  %0 = call i64 @llvm.read_register.i64(metadata !3)
  %1 = inttoptr i64 %0 to ptr
  %2 = getelementptr inbounds i8, ptr %1, i64 12
  %3 = load i16, ptr %2, align 2
  %need_safepoint = icmp ne i16 %3, 0
  br i1 %need_safepoint, label %safepoint, label %7, !prof !4

safepoint:                                        ; preds = %bb
  %4 = call i64 @llvm.read_register.i64(metadata !3)
  %5 = inttoptr i64 %4 to ptr
  %6 = getelementptr inbounds i8, ptr %5, i64 208
  %__panda_entrypoint_Safepoint_addr = load ptr, ptr %6, align 8
  call void %__panda_entrypoint_Safepoint_addr() #3
  br label %7

7:                                                ; preds = %safepoint, %bb
  ret void
}

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(read)
declare i64 @llvm.read_register.i64(metadata) #1

; Function Attrs: nounwind
define protected i32 @"i32 _GLOBAL::main()_id_3464346901858484365"(ptr nonnull %method) #2 gc "ark" !section_prefix !5 !class_id !6 !use-ark-frame !7 {
bb1:
  br label %bb0

bb0:                                              ; preds = %bb1
  ret i32 0
}

attributes #0 = { nounwind }
attributes #1 = { nocallback nofree nosync nounwind willreturn memory(read) }
attributes #2 = { nounwind "frame-pointer"="all" }
attributes #3 = { "safepoint" }

!ark.frame.info = !{!0, !1, !2}

!0 = !{i32 0}
!1 = !{i32 -120, i32 -128, i32 -160, i32 -168, i32 -112, i32 -136}
!2 = !{i32 320}
!3 = !{!"r15"}
!4 = !{!"branch_weights", i32 1, i32 2000}
!5 = !{!"function_section_prefix", !"i32 _GLOBAL::main()_id_3464346901858484365"}
!6 = !{i32 122}
!7 = !{}
