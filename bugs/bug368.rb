Shoes.app do
list = 1..20
@new_list = []
    
    check(checked: false) { |c| @menu.contents.each { |f| f.contents[0].checked = c.checked? ? true : false }}
    @menu = flow left: 0, top: 50, width: 100, height: 400 do
         list.each_with_index do |n, i| 
               flow width: 0.8, height: 30 do ###################This line is different
               #flow width: 0.8 do ###################This line is different
                    check(checked: false) { |cc| cc.checked? ?  @new_list.push(n) : @new_list.delete(n) }
                    para n
                    border red
               end
         end
    end
debug(@new_list = [])
end
