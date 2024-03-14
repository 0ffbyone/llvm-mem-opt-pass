; ModuleID = './IR/no-opt-three.ll'
source_filename = "no-opt-three.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

; Function Attrs: nounwind sspstrong uwtable
define dso_local i32 @foo(i32 noundef %0, i32 noundef %1) local_unnamed_addr #0 {
  %3 = tail call noalias dereferenceable_or_null(1024) ptr @malloc(i64 noundef 1024) #3
  switch i32 %0, label %6 [
    i32 50, label %4
    i32 42, label %5
  ], !prof !5

4:                                                ; preds = %2
  tail call void @bar(ptr noundef %3) #4
  br label %6

5:                                                ; preds = %2
  tail call void (...) @foo_bar() #4
  br label %6

6:                                                ; preds = %5, %4, %2
  switch i32 %1, label %9 [
    i32 50, label %7
    i32 42, label %8
  ], !prof !5

7:                                                ; preds = %6
  tail call void @bar(ptr noundef %3) #4
  br label %9

8:                                                ; preds = %6
  tail call void (...) @func() #4
  br label %9

9:                                                ; preds = %8, %7, %6
  ret i32 0
}

; Function Attrs: mustprogress nofree nounwind willreturn allockind("alloc,uninitialized") allocsize(0) memory(inaccessiblemem: readwrite)
declare noalias noundef ptr @malloc(i64 noundef) local_unnamed_addr #1

declare void @bar(ptr noundef) local_unnamed_addr #2

declare void @foo_bar(...) local_unnamed_addr #2

declare void @func(...) local_unnamed_addr #2

attributes #0 = { nounwind sspstrong uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { mustprogress nofree nounwind willreturn allockind("alloc,uninitialized") allocsize(0) memory(inaccessiblemem: readwrite) "alloc-family"="malloc" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #2 = { "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #3 = { nounwind allocsize(0) }
attributes #4 = { nounwind }

!llvm.module.flags = !{!0, !1, !2, !3}
!llvm.ident = !{!4}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 8, !"PIC Level", i32 2}
!2 = !{i32 7, !"PIE Level", i32 2}
!3 = !{i32 7, !"uwtable", i32 2}
!4 = !{!"clang version 16.0.6"}
!5 = !{!"branch_weights", i32 4000000, i32 2001, i32 2000}
