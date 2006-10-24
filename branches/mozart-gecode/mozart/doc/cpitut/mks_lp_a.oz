declare
fun {SelectVar VsPair}
   case VsPair
   of nil#nil then unit#unit 
   [] (VH|VT)#(RVH|RVT) then
      % check for integrality
      case RVH == {Round RVH}
      then {SelectVar VT#RVT}
      else VH#RVH end
   else unit
   end
end   
