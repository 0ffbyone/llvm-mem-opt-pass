; =========================================
; LLVM IR module BEFORE LLVM optimizations:
; =========================================
; ModuleID = 'panda-llvmaot-module'
source_filename = "panda-llvmaot-module"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@__aot_got = protected global [0 x i64] zeroinitializer, section ".aot_got"

; Function Attrs: nounwind
define void @gc.safepoint_poll() #0 {
bb:
  %0 = call i64 @llvm.read_register.i64(metadata !5)
  %1 = inttoptr i64 %0 to ptr
  %2 = getelementptr inbounds i8, ptr %1, i64 12
  %3 = load i16, ptr %2, align 2
  %need_safepoint = icmp ne i16 %3, 0
  br i1 %need_safepoint, label %safepoint, label %7, !prof !6

safepoint:                                        ; preds = %bb
  %4 = call i64 @llvm.read_register.i64(metadata !5)
  %5 = inttoptr i64 %4 to ptr
  %6 = getelementptr inbounds i8, ptr %5, i64 208
  %__panda_entrypoint_Safepoint_addr = load ptr, ptr %6, align 8
  call void %__panda_entrypoint_Safepoint_addr() #3
  br label %7

7:                                                ; preds = %bb, %safepoint
  ret void
}

; Function Attrs: nounwind readonly
declare i64 @llvm.read_register.i64(metadata) #1

; Function Attrs: nounwind
define protected i32 @"i32 _GLOBAL::main()_id_3464346901858484365"(ptr nonnull %method) #2 gc "ark" !dbg !7 !section_prefix !11 !class_id !12 !use-ark-frame !10 {
bb1:
  br label %bb0

bb0:                                              ; preds = %bb1
  ret i32 0
}

attributes #0 = { nounwind }
attributes #1 = { nounwind readonly }
attributes #2 = { nounwind "frame-pointer"="all" }
attributes #3 = { "safepoint" }

!llvm.dbg.cu = !{!0}
!ark.frame.info = !{!2, !3, !4}

!0 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus_14, file: !1, producer: "ark-llvm-backend", isOptimized: true, runtimeVersion: 0, emissionKind: NoDebug, nameTableKind: None)
!1 = !DIFile(filename: "/home/tmp/build/tests/irtoc-opcode-suite-llvmaot-gen-gc/fneg.pa-llvmaot-gen-gc-execute/test.an", directory: "/")
!2 = !{i32 0}
!3 = !{i32 -120, i32 -128, i32 -160, i32 -168, i32 -112, i32 -136}
!4 = !{i32 320}
!5 = !{!"r15"}
!6 = !{!"branch_weights", i32 1, i32 2000}
!7 = distinct !DISubprogram(name: "i32 _GLOBAL::main()_id_3464346901858484365", linkageName: "i32 _GLOBAL::main()_id_3464346901858484365", scope: !8, file: !8, line: 141, type: !9, scopeLine: 141, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !0, retainedNodes: !10)
!8 = !DIFile(filename: "/home/tmp/build/tests/irtoc-opcode-suite-llvmaot-gen-gc/fneg.pa-llvmaot-gen-gc-execute/test.abc", directory: "/")
!9 = !DISubroutineType(types: !10)
!10 = !{}
!11 = !{!"function_section_prefix", !"i32 _GLOBAL::main()_id_3464346901858484365"}
!12 = !{i32 122}

