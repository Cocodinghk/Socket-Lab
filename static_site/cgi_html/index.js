let URL = "127.0.0.0:9999";

$(".button").on('click', () => {
    let name = $(".name").val();
    let password = $(".password").val();

    console.log(name, password)

    let params = {
        name: name,
        password: password
    }

    fetch(URL, {
        method: 'post',
        body: JSON.stringify(params)
    }).then(res => {
        if (res.status == 200) {
            res.json().then(data=>{StandardPost(data.content)})
        }
        else console.log("Failure", res.status);
    })
        .catch(err => {
            console.log(err)
        })
})

function StandardPost(html) { //先json转换，然后把内容传到函数中，存到本地缓存
    localStorage.removeItem('callbackHTML');
    localStorage.setItem('callbackHTML',html);
    window.location.href = "/cgi_html/dynamic.html"; //靠新的get：利用html脚本把本地缓存中的内容取出来（刷新页面）
}