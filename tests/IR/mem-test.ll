; ModuleID = 'mem-test.c'
source_filename = "mem-test.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

; Function Attrs: noinline nounwind optnone sspstrong uwtable
define dso_local void @my_malloc() #0 {
  %1 = alloca ptr, align 8
  %2 = alloca ptr, align 8
  %3 = call i64 @time(ptr noundef null) #4
  %4 = trunc i64 %3 to i32
  call void @srand(i32 noundef %4) #4
  %5 = call noalias ptr @malloc(i64 noundef 40) #5
  store ptr %5, ptr %1, align 8
  %6 = call i32 @rand() #4
  %7 = icmp eq i32 %6, 25789432
  br i1 %7, label %8, label %11

8:                                                ; preds = %0
  %9 = load ptr, ptr %1, align 8
  %10 = getelementptr inbounds i32, ptr %9, i64 0
  store i32 1, ptr %10, align 4
  br label %12

11:                                               ; preds = %0
  br label %12

12:                                               ; preds = %11, %8
  %13 = call noalias ptr @calloc(i64 noundef 1, i64 noundef 4) #6
  store ptr %13, ptr %2, align 8
  %14 = load ptr, ptr %2, align 8
  call void @free(ptr noundef %14) #4
  %15 = load ptr, ptr %1, align 8
  call void @free(ptr noundef %15) #4
  ret void
}

; Function Attrs: nounwind
declare void @srand(i32 noundef) #1

; Function Attrs: nounwind
declare i64 @time(ptr noundef) #1

; Function Attrs: nounwind allocsize(0)
declare noalias ptr @malloc(i64 noundef) #2

; Function Attrs: nounwind
declare i32 @rand() #1

; Function Attrs: nounwind allocsize(0,1)
declare noalias ptr @calloc(i64 noundef, i64 noundef) #3

; Function Attrs: nounwind
declare void @free(ptr noundef) #1

; Function Attrs: noinline nounwind optnone sspstrong uwtable
define dso_local i32 @main() #0 {
  %1 = alloca i32, align 4
  %2 = alloca ptr, align 8
  %3 = alloca i32, align 4
  store i32 0, ptr %1, align 4
  %4 = call noalias ptr @malloc(i64 noundef 4) #5
  store ptr %4, ptr %2, align 8
  call void @my_malloc()
  store i32 5, ptr %3, align 4
  %5 = load ptr, ptr %2, align 8
  call void @free(ptr noundef %5) #4
  ret i32 0
}

attributes #0 = { noinline nounwind optnone sspstrong uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { nounwind "frame-pointer"="all" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #2 = { nounwind allocsize(0) "frame-pointer"="all" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #3 = { nounwind allocsize(0,1) "frame-pointer"="all" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #4 = { nounwind }
attributes #5 = { nounwind allocsize(0) }
attributes #6 = { nounwind allocsize(0,1) }

!llvm.module.flags = !{!0, !1, !2, !3, !4}
!llvm.ident = !{!5}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 8, !"PIC Level", i32 2}
!2 = !{i32 7, !"PIE Level", i32 2}
!3 = !{i32 7, !"uwtable", i32 2}
!4 = !{i32 7, !"frame-pointer", i32 2}
!5 = !{!"clang version 16.0.6"}
