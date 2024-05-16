; ModuleID = './tests/c-sources/no-bb-eight.c'
source_filename = "./tests/c-sources/no-bb-eight.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

; Function Attrs: nounwind sspstrong uwtable
define dso_local i32 @foo(i32 noundef %0, i32 noundef %1) local_unnamed_addr #0 {
  %3 = call noalias dereferenceable_or_null(1024) i8* @malloc(i64 noundef 1024) #3
  %4 = and i32 %0, 1
  %5 = icmp eq i32 %4, 0
  br i1 %5, label %10, label %6

6:                                                ; preds = %2
  %7 = icmp eq i32 %1, 42
  br i1 %7, label %8, label %9, !prof !5

8:                                                ; preds = %6
  call void @bar(i8* noundef %3) #3
  br label %14

9:                                                ; preds = %6
  call void (...) @func() #3
  br label %14

10:                                               ; preds = %2
  %11 = icmp eq i32 %1, 420
  br i1 %11, label %12, label %13, !prof !5

12:                                               ; preds = %10
  call void @bar(i8* noundef %3) #3
  br label %14

13:                                               ; preds = %10
  call void (...) @foo_bar() #3
  br label %14

14:                                               ; preds = %12, %13, %8, %9
  ret i32 0
}

; Function Attrs: inaccessiblememonly mustprogress nofree nounwind willreturn
declare noalias noundef i8* @malloc(i64 noundef) local_unnamed_addr #1

declare void @bar(i8* noundef) local_unnamed_addr #2

declare void @func(...) local_unnamed_addr #2

declare void @foo_bar(...) local_unnamed_addr #2

attributes #0 = { nounwind sspstrong uwtable "frame-pointer"="none" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { inaccessiblememonly mustprogress nofree nounwind willreturn "frame-pointer"="none" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #2 = { "frame-pointer"="none" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #3 = { nounwind }

!llvm.module.flags = !{!0, !1, !2, !3}
!llvm.ident = !{!4}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"PIC Level", i32 2}
!2 = !{i32 7, !"PIE Level", i32 2}
!3 = !{i32 7, !"uwtable", i32 1}
!4 = !{!"clang version 14.0.6"}
!5 = !{!"branch_weights", i32 1, i32 2000}
