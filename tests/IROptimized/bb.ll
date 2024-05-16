; ModuleID = './tests/IR/bb.ll'
source_filename = "./tests/IR/bb.ll"

define ptr @foo(i32 %x, i32 %y) {
entry:
  %bool_x = icmp eq i32 %x, 42
  %bool_y = icmp eq i32 %y, 42
  br i1 %bool_x, label %first, label %second

first:                                            ; preds = %entry
  br i1 %bool_y, label %unlikely, label %first_likely, !prof !0

second:                                           ; preds = %entry
  br i1 %bool_y, label %unlikely, label %second_likely, !prof !0

first_likely:                                     ; preds = %first
  ret ptr null

second_likely:                                    ; preds = %second
  ret ptr null

unlikely:                                         ; preds = %second, %first
  %allocated = tail call noalias dereferenceable_or_null(1024) ptr @malloc(i64 noundef 1024)
  tail call void @bar(ptr noundef %allocated)
  br label %retlabel

retlabel:                                         ; preds = %unlikely
  ret ptr null
}

; Function Attrs: mustprogress nofree nounwind willreturn allockind("alloc,uninitialized") allocsize(0) memory(inaccessiblemem: readwrite)
declare noalias noundef ptr @malloc(i64 noundef) local_unnamed_addr #0

declare void @bar(ptr noundef) local_unnamed_addr #1

attributes #0 = { mustprogress nofree nounwind willreturn allockind("alloc,uninitialized") allocsize(0) memory(inaccessiblemem: readwrite) "alloc-family"="malloc" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }

!0 = !{!"branch_weights", i32 1, i32 2000}
