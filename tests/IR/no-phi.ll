; ModuleID = 'phi.ll'

define ptr @foo(i32 %x, i32 %y) {
entry:
  %allocated = tail call noalias dereferenceable_or_null(1024) ptr @malloc(i64 noundef 1024) #3
  %bool_x = icmp eq i32 %x, 42
  %bool_y = icmp eq i32 %y, 42
  br i1 %bool_x, label %first, label %second

first:
  br i1 %bool_y, label %unlikely, label %likely, !prof !0

second:
  br i1 %bool_y, label %unlikely, label %likely, !prof !0

likely:
  ret ptr null

unlikely:
  %0 = phi ptr [ %allocated, %first ], [ null, %second ]
  br label %retlabel

retlabel:
  ret ptr null
}

; Function Attrs: mustprogress nofree nounwind willreturn allockind("alloc,uninitialized") allocsize(0) memory(inaccessiblemem: readwrite)
declare noalias noundef ptr @malloc(i64 noundef) local_unnamed_addr #1


attributes #1 = { mustprogress nofree nounwind willreturn allockind("alloc,uninitialized") allocsize(0) memory(inaccessiblemem: readwrite) "alloc-family"="malloc" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }


!0 = !{!"branch_weights", i32 1, i32 2000}
